//
// Created by luoxj on 11/3/20.
//

#ifndef ROOT_TOOLKIT_GETCUTRANGE_AND_ISSPE_BIGPEAK_HH
#define ROOT_TOOLKIT_GETCUTRANGE_AND_ISSPE_BIGPEAK_HH
#include "TH1D.h"
#include "TString.h"
#include "TF1.h"
#include "pars_waves.hh"
#include "type_transform.hh"
#include "plot.hh"
#include "TH1_tool.hh"
#include "GetWavePeakNum.hh"

int count_NumOfWaves_InCertainNPeak(const vector<int> v_NPeak, const int CertainPeakNum)
{
    int n_count=0;
    for(int i=0;i<v_NPeak.size();i++)
    {
        if (v_NPeak[i]==CertainPeakNum)
        {
            n_count++;
        }
    }
    return n_count;
}
double IntegralRange(TH1D* h_wave, const int bin_start, const int bin_end, double baseline)
{
    double Integral=0;
    for (int i = bin_start; i < bin_end; i++)
    {
        Integral+=baseline-h_wave->GetBinContent(i);
    }
    return Integral;

}
bool check_NBinsToGetBaseline(TH1D* h_wave, const int n_BinToGetBaseline, const double threshold_checkBaseline)
{
    double max=Max_subbin(h_wave,0,n_BinToGetBaseline)[1];
    double min=Min_subbin(h_wave,0,n_BinToGetBaseline)[1];
    if ( abs(max-min)>threshold_checkBaseline )
    {
        return false;  //The flutuation of the beginning bins is too large
    }
    else
    {
        return true;
    }
}

//this function is to fill  the integral of big peak waves into Hist, so it has used threshold 50
vector<double> getCutRange_and_isSPE_BigPeak( TString nameFile){
    pars_waves parsWaves;
    bool debug=false;
    bool plot_selected_result=false;
    const double times_rms=2.5;
    bool plot_waves_divide=false;
    bool plot_check_IsSPE_to_pdf=true;
    bool useThereshold50 = true;
    TString name_option="";
    if (debug==true)
    {
        name_option.Append("_debug");
    }
    bool single_peak_fit=true;
    bool Integral_onePeak=true;
    system("mkdir -p " + parsWaves.name_PdfDir + "chargeHist/");


    TString dir="";

    int n_bin_getBaseline=parsWaves.n_bin_getBaseline;
    const int nDimension = parsWaves.nDimension;
    int tmp_sigma_selectSinglePhoton = parsWaves.sigma_selectSinglePhoton;
    double sigma_selectSinglePhoton = (double)tmp_sigma_selectSinglePhoton/10.;

    TString newname=nameFile;
    dir.Append(newname);
    dir.Append("_RawData.root");
    TFile* f=new TFile(parsWaves.name_RootFilePath + dir, "read");


    //TFile* f=new TFile("new1divide.root","read");
    TTree* tr=(TTree*)f->Get("SignalBigPeak");
    TTree* tr_waves=(TTree*)f->Get("waves");
    double entry_waves=tr_waves->GetEntries();

    int entry=tr->GetEntries();
    TH1D* isSPE=new TH1D("isSPE","isSPE",entry,0,entry);
    TH1D* waveform=NULL;
    TH1D* chargeHist=new TH1D("chargeHist","chargeHist",200,00,10000);
    // TH1D* chargeHist=new TH1D("chargeHist","chargeHist",200,00,5000);
    double baseline=0;
    double rms=0;
    double limit=0;
    int Integral=0;
    tr->SetBranchAddress("SignalBigPeak",&waveform);

    vector<double> v_integral;
    vector<int> v_PeakNum; //To record the num of peaks in every wave
    vector<bool> v_check_NBinToGetBaseline;//To check The biginning 50 bins whether they exist a peak
    double threshold_CheckBaseline=50;

    if ( plot_waves_divide==true )
    {
        for (int k = 0; k < 10; k++)
        {
            tr->GetEntry(k);
            TCanvas *c1=new TCanvas("waves"+(TString)n2str(k),"waves"+(TString)n2str(k),800,600);
            waveform->DrawCopy();
        }
    }
    vector<TH1D*> v_2Peak_toPdf;
    int n_2Peak_toPdf=0;

    if ( debug==true )	entry=10;

    for (int i=0;i<entry;i++){
        tr->GetEntry(i);
        // waveform->Scale(-1.);

        baseline=0;
        rms=0;
        limit=0;
        Integral=0;

        bool whether_NBinToGetBaseline_isReliable=check_NBinsToGetBaseline(waveform, parsWaves.n_bin_getBaseline, threshold_CheckBaseline);
        v_check_NBinToGetBaseline.push_back(whether_NBinToGetBaseline_isReliable);
        if (whether_NBinToGetBaseline_isReliable==false && debug==false)
        {
            v_integral.push_back(-100);
            v_PeakNum.push_back(-1);
            continue;
        }


        if (debug ==true && i<20)
        {
            TCanvas *c1=new TCanvas("waves"+(TString)n2str(i),"waves"+(TString)n2str(i),800,600);
            waveform->DrawCopy();
            bool whether_NBinToGetBaseline_isReliable=check_NBinsToGetBaseline(waveform, parsWaves.n_bin_getBaseline, threshold_CheckBaseline);
            cout<< i<<"  Whether reliable?  "<<whether_NBinToGetBaseline_isReliable<<endl;
            cout << i << "  N_Peak:   " << GetWaveValleyNum(waveform, parsWaves.n_bin_getBaseline, baseline) << endl;
        }

        int n_Peak=GetWaveValleyNum(waveform, parsWaves.n_bin_getBaseline, baseline );
        v_PeakNum.push_back(n_Peak);
        if (n_Peak==2 && n_2Peak_toPdf<80 )
        {
            TString name_2Peak="waveform_2Peak";
            v_2Peak_toPdf.push_back((TH1D*)waveform->Clone(name_2Peak+i));
            n_2Peak_toPdf++;
        }


        if(Integral_onePeak==true)
        {
            for (int j=0;j<n_bin_getBaseline;j++){
                rms+=(waveform->GetBinContent(j+1)-baseline)*(waveform->GetBinContent(j+1)-baseline);
            }
            rms=sqrt((double)rms/n_bin_getBaseline);
            //	 cout<<baseline<<","<<rms<<endl;
            // cout<< "Max:  " <<waveform->GetMaximum() << " Min: " <<waveform->GetMinimum()<<endl;
            // if ( abs(waveform->GetMaximum()-baseline) >50 ) continue;
            limit=baseline+rms*times_rms;
            // limit=baseline+50;

            //注意！！！这里的waveform是电子学的读出没有转化，所以得到波形的峰是向下的，因此这里一定一定要注意，不是用波形减去基线，而是基线减去波形
            for (int j=0;j<nDimension;j++)
            {
                if (2*baseline-(double)waveform->GetBinContent(j+1)>limit){
                    Integral+=baseline-waveform->GetBinContent(j+1);
                }

            }
            // cout<< i << "    baseline:   "<< baseline <<endl;
            //	cout<<Integral<<","<<endl;

        }
        else
        {
            Integral=IntegralRange(waveform, parsWaves.n_bin_getBaseline - 10, parsWaves.n_bin_getBaseline + 50, baseline);
            // cout<<i<<"  Integral=   "<< Integral<<endl;
        }
        if( n_Peak==1 )
        {
            chargeHist->Fill(Integral);
        }
        // cout<<Integral<<endl;
        v_integral.push_back(Integral);
    }



    if ( debug==true )
    {
        TCanvas *c1=new TCanvas("chargeHist","chargeHist",800,600);
        chargeHist->DrawCopy();
    }


    int min=0;
    int binmin=10000000;
    for (int j=0;j<20;j++){
        if (chargeHist->GetBinContent(j+1)<binmin) {
            binmin=chargeHist->GetBinContent(j+1);
            min=j+1;
        }
    }
    min=min*100;
    cout<<"min="<<min<<endl;
    int found=0;
    int min2=0;
    int binmin2=10000000;
    for (int j=20;j<200;j++){
        if (found==0 && chargeHist->GetBinContent(j+1)<binmin) {
            binmin2=chargeHist->GetBinContent(j+1);
            min2=j+1;
            found=1;
        }
    }
    min2=min2*100;
    cout<<"min2="<<min2<<endl;


    TCanvas* can8=new TCanvas("SPE"+(TString) n2str(times_rms),"SPE"+(TString) n2str(times_rms),800,600);
    //chargeHist->Fit("gaus");
    TF1* fun1=NULL;

    vector<double> v_range(2);
    double low=0;
    double high=0;
    if ( single_peak_fit == false )
    {
        fun1=new TF1("fun1","gaus",min,min2);
        // fun1=new TF1("fun1","gaus",500,2000);
        chargeHist->Fit("fun1","R");
        low=fun1->GetParameter(1)- sigma_selectSinglePhoton*fun1->GetParameter(2);
        high=fun1->GetParameter(1)+ sigma_selectSinglePhoton* fun1->GetParameter(2);
    }
    else
    {
        cout<< "binCenter:   " << chargeHist->GetBinCenter(3)<<"   "<<chargeHist->GetBinCenter(200)<<endl;
        // TH1D* chargeHist_tmp=new TH1D("h_tmp","h_tmp",200,0,20000);
        // for(int j=4;j<10;j++)
        // {
        // 	chargeHist_tmp->SetBinContent(j,chargeHist_tmp->GetBinContent(j));
        // }
        // TCanvas *c_tmp=new TCanvas("waves_tmp","charge_tmp",800,600);
        // chargeHist_tmp->DrawCopy();
        chargeHist->SetBinContent(0,0);
        chargeHist->SetBinContent(1,0);
        chargeHist->SetBinContent(2,0);
        double max_h = chargeHist->GetBinCenter(chargeHist->GetMaximumBin());
        // double max_h = Max_subbin(chargeHist, 2, 50)[0];
        cout<<"max loc:  "<<max_h<< "  value:   "<<chargeHist->GetBinContent(chargeHist->GetMaximumBin())<<endl;
        fun1=new TF1("fun1","gaus",max_h-500, max_h+500 );
        chargeHist->Fit("fun1","R");
        low=fun1->GetParameter(1)-sigma_selectSinglePhoton * fun1->GetParameter(2);
        high=fun1->GetParameter(1)+sigma_selectSinglePhoton *fun1->GetParameter(2);
        cout<< "high=  "<<high<<endl;
    }
    v_range[0]=low;
    v_range[1] =high;

    chargeHist->DrawCopy();
    fun1->DrawCopy("same");

    //chargeHist->Fit("gaus","Q");
    //TF1* fun1=chargeHist->GetFunction("gaus");
    //cout<<fun1->GetParameter(1)<<" and "<<fun1->GetParameter(2)<<endl;
    if ( single_peak_fit == false )
    {
        if (low<min) low=min;
        if (high<min || low>min2) cout<<"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"<<endl;
        if (high>min2) high=min2;
    }
    cout<<"low="<<low<<" high="<<high<<endl;
    delete fun1;
    int countspe=0;
    for (int i=0;i<entry;i++){

        Integral = v_integral[i];
        // cout<< "Integral: "<<Integral<<endl;
        // cout<< "high and low:  "<<high<<"   "<<low<<endl;
        if (Integral>low &&Integral<high && v_PeakNum[i]==1 && v_check_NBinToGetBaseline[i]==true)
        {
            isSPE->SetBinContent(i+1,1);
            countspe++;
        }
        //                                                                                                      cout<<Integral<<","<<endl;
    }
    if (plot_check_IsSPE_to_pdf == true)
    {
        const int n_line_to_pdf = 10;
        vector<vector<TH1D*>> v2D_signal_TH1D_toPDF( n_line_to_pdf );
        int n_to_pdf_waves=0;
        int n_c=0;
        for (int i = 0; i < entry ; i++)
        {
            tr->GetEntry(i);
            //注意！！！使用isSPE时，记住它是从1开始的，所以时刻记住要用（i+1）去提取数据
            if( isSPE->GetBinContent(i+1) ==1 )
            {
                waveform->SetBinContent(parsWaves.nDimension, waveform->GetBinContent(parsWaves.nDimension - 3));
                // waveform->SetBinContent(0, waveform->GetBinContent(parsWaves.nDimension-3));
                // cout<<i<<endl;
                waveform->SetXTitle("Time (ns)");
                waveform->SetYTitle("ADC");
                v2D_signal_TH1D_toPDF[n_to_pdf_waves/4].push_back(
                        (TH1D*) waveform->Clone( (TString)n2str(i)+"h_waveform_isSPE="+(TString)n2str( isSPE->GetBinContent(i+1) ) ));
                n_to_pdf_waves++;
                if(n_to_pdf_waves==n_line_to_pdf*4)break;
                if( n_c<15 && plot_selected_result==true )
                {
                    TCanvas *c1=new TCanvas("waves"+(TString)n2str(i),"waves"+(TString)n2str(i),800,600);
                    waveform->DrawCopy();
                    bool whether_NBinToGetBaseline_isReliable=check_NBinsToGetBaseline(waveform, parsWaves.n_bin_getBaseline, threshold_CheckBaseline);
                    cout<< i<<"  Whether reliable?  "<<whether_NBinToGetBaseline_isReliable<<endl;
                    cout << i << "  N_Peak:   " << GetWaveValleyNum(waveform, parsWaves.n_bin_getBaseline, baseline) << endl;
                }
                n_c++;
            }
        }

        if ( useThereshold50 == true )
        {
            name_option.Append("_useThreshold50");
        }
        else
        {
            name_option.Append("NoUseThreshold50");
        }

        name_option.Append("_"+(TString)n2str(sigma_selectSinglePhoton)+"SelectSigma");

        plot_into_pdf(v2D_signal_TH1D_toPDF, parsWaves.name_PdfDir + "chargeHist/" + newname + "_checkSinglePhotonWaves" + name_option + ".pdf");
        for (int z = 0; z < v2D_signal_TH1D_toPDF.size(); z++)
        {
            //释放内存
            vector<TH1D*>().swap(v2D_signal_TH1D_toPDF[z]);
        }

    }

    TString name0=newname;
    name0.Append("_check");
    if( useThereshold50== true )
    {
        name0.Append("_useThreshold50");
    }
    else
    {
        name0.Append("_NoUseThreshold");
    }
    if (debug==true)
    {
        name0.Append("_debug");
    }
    TFile* g=new TFile(parsWaves.name_RootFilePath + name0 + ".root", "recreate");
    //TFile* g=new TFile("SPEimage.root","recreate");
    g->cd();
    cout<<"entry="<<entry<<" spe="<<countspe<<endl;
    isSPE->Write();
    chargeHist->Write();

    double n_1Peak=count_NumOfWaves_InCertainNPeak(v_PeakNum, 1);
    double n_2Peak=count_NumOfWaves_InCertainNPeak(v_PeakNum, 2);
    double n_NoCount=count_NumOfWaves_InCertainNPeak(v_PeakNum, -1);
    double ratio_2PeakTo1Peak=n_2Peak/n_1Peak;
    double ratio_1PeakToEffectiveEvent=n_1Peak/(v_PeakNum.size());
    double ratio_1PeakToAllWaves=n_1Peak/entry_waves*2 ;

    TCanvas* c_chargeHist=new TCanvas("c_chargeHist","c_chargeHist",800,600);
    chargeHist->SetTitle("ChargeHist_SelectRange:"+n2str(low)+"---"+n2str(high)+
                         ",NSelectedWaves:"+n2str(countspe)+",1Peak N:"+n2str(n_1Peak)
                         +",Ratio 2PeakTo1:"+n2str(ratio_2PeakTo1Peak)+",1PeakToAll:"+n2str(ratio_1PeakToEffectiveEvent));
    chargeHist->SetXTitle("Integral");
    chargeHist->SetYTitle("N");
    chargeHist->DrawCopy();
    c_chargeHist->SetLogy();

    // ofstream output1;
    // output1.open(parsWaves.name_PdfDir+"Ratio_txt/"+"Ratio.txt");
    TString str_ToRatioTxt=newname+",";
    str_ToRatioTxt+=n2str(ratio_1PeakToEffectiveEvent)+","+n2str(ratio_2PeakTo1Peak)+","+n2str(ratio_1PeakToAllWaves)
                    +","+n2str(countspe)+","+n2str((double)countspe/(double)entry);
    system("mkdir -p " + parsWaves.name_PdfDir + "Ratio_txt/");
    system("echo '" + str_ToRatioTxt + "'>> " + parsWaves.name_PdfDir + "Ratio_txt/" + "Ratio.txt");
    system("mkdir -p " + parsWaves.name_PdfDir + "Entries_Info/");
    system("echo '" + n2str(entry) + "'>" + parsWaves.name_PdfDir + "Entries_Info/" + newname + ".txt");




    c_chargeHist->SaveAs(parsWaves.name_PdfDir + "chargeHist/" + newname + "_ChargeHist" + name_option + ".png");
    plot_into_pdf(v_2Peak_toPdf , parsWaves.name_PdfDir + "chargeHist/" + newname + "_Check2Peak" + name_option + ".pdf");
    // TCanvas* can1=new TCanvas("c2","c2",800,600);
    // can1->cd();
    // chargeHist->DrawCopy();

    g->Close();
    f->Close();
    return v_range;
}

//this function is to fill  the integral of big peak waves into Hist, and we need to extract SPE from PDE data therefore it is needed to get v_range
void getCutRange_and_isSPE_BigPeak( TString nameFile, vector<double> v_range){
    pars_waves parsWaves;
    bool debug=false;
    bool plot_selected_result=false;
    const double times_rms=2.5;
    bool plot_waves_divide=false;
    bool plot_check_IsSPE_to_pdf=true;
    bool useThereshold50 = true;
    TString name_option="";
    if (debug==true)
    {
        name_option.Append("_debug");
    }
    bool single_peak_fit=true;
    bool Integral_onePeak=true;
    system("mkdir -p " + parsWaves.name_PdfDir + "chargeHist/");


    TString dir="";

    int n_bin_getBaseline=parsWaves.n_bin_getBaseline;
    const int nDimension = parsWaves.nDimension;
    int tmp_sigma_selectSinglePhoton = parsWaves.sigma_selectSinglePhoton;
    double sigma_selectSinglePhoton = (double)tmp_sigma_selectSinglePhoton/10.;

    TString newname=nameFile;
    dir.Append(newname);
    dir.Append("_RawData.root");
    TFile* f=new TFile(parsWaves.name_RootFilePath + dir, "read");


    //TFile* f=new TFile("new1divide.root","read");
    TTree* tr=(TTree*)f->Get("SignalBigPeak");
    TTree* tr_waves=(TTree*)f->Get("waves");
    double entry_waves=tr_waves->GetEntries();

    int entry=tr->GetEntries();
    TH1D* isSPE=new TH1D("isSPE","isSPE",entry,0,entry);
    TH1D* waveform=NULL;
    TH1D* chargeHist=new TH1D("chargeHist","chargeHist",200,00,10000);
    // TH1D* chargeHist=new TH1D("chargeHist","chargeHist",200,00,5000);
    double baseline=0;
    double rms=0;
    double limit=0;
    int Integral=0;
    tr->SetBranchAddress("SignalBigPeak",&waveform);

    vector<double> v_integral;
    vector<int> v_PeakNum; //To record the num of peaks in every wave
    vector<bool> v_check_NBinToGetBaseline;//To check The biginning 50 bins whether they exist a peak
    double threshold_CheckBaseline=50;

    if ( plot_waves_divide==true )
    {
        for (int k = 0; k < 10; k++)
        {
            tr->GetEntry(k);
            TCanvas *c1=new TCanvas("waves"+(TString)n2str(k),"waves"+(TString)n2str(k),800,600);
            waveform->DrawCopy();
        }
    }
    vector<TH1D*> v_2Peak_toPdf;
    int n_2Peak_toPdf=0;

    if ( debug==true )	entry=10;

    for (int i=0;i<entry;i++){
        tr->GetEntry(i);
        // waveform->Scale(-1.);

        baseline=0;
        rms=0;
        limit=0;
        Integral=0;

        bool whether_NBinToGetBaseline_isReliable=check_NBinsToGetBaseline(waveform, parsWaves.n_bin_getBaseline, threshold_CheckBaseline);
        v_check_NBinToGetBaseline.push_back(whether_NBinToGetBaseline_isReliable);
        if (whether_NBinToGetBaseline_isReliable==false && debug==false)
        {
            v_integral.push_back(-100);
            v_PeakNum.push_back(-1);
            continue;
        }


        if (debug ==true && i<20)
        {
            TCanvas *c1=new TCanvas("waves"+(TString)n2str(i),"waves"+(TString)n2str(i),800,600);
            waveform->DrawCopy();
            bool whether_NBinToGetBaseline_isReliable=check_NBinsToGetBaseline(waveform, parsWaves.n_bin_getBaseline, threshold_CheckBaseline);
            cout<< i<<"  Whether reliable?  "<<whether_NBinToGetBaseline_isReliable<<endl;
            cout << i << "  N_Peak:   " << GetWaveValleyNum(waveform, parsWaves.n_bin_getBaseline, baseline) << endl;
        }

        int n_Peak=GetWaveValleyNum(waveform, parsWaves.n_bin_getBaseline, baseline );
        v_PeakNum.push_back(n_Peak);
        if (n_Peak==2 && n_2Peak_toPdf<80 )
        {
            TString name_2Peak="waveform_2Peak";
            v_2Peak_toPdf.push_back((TH1D*)waveform->Clone(name_2Peak+i));
            n_2Peak_toPdf++;
        }


        if(Integral_onePeak==true)
        {
            for (int j=0;j<n_bin_getBaseline;j++){
                rms+=(waveform->GetBinContent(j+1)-baseline)*(waveform->GetBinContent(j+1)-baseline);
            }
            rms=sqrt((double)rms/n_bin_getBaseline);
            //	 cout<<baseline<<","<<rms<<endl;
            // cout<< "Max:  " <<waveform->GetMaximum() << " Min: " <<waveform->GetMinimum()<<endl;
            // if ( abs(waveform->GetMaximum()-baseline) >50 ) continue;
            limit=baseline+rms*times_rms;
            // limit=baseline+50;

            //注意！！！这里的waveform是电子学的读出没有转化，所以得到波形的峰是向下的，因此这里一定一定要注意，不是用波形减去基线，而是基线减去波形
            for (int j=0;j<nDimension;j++)
            {
                if (2*baseline-(double)waveform->GetBinContent(j+1)>limit){
                    Integral+=baseline-waveform->GetBinContent(j+1);
                }

            }
            // cout<< i << "    baseline:   "<< baseline <<endl;
            //	cout<<Integral<<","<<endl;

        }
        else
        {
            Integral=IntegralRange(waveform, parsWaves.n_bin_getBaseline - 10, parsWaves.n_bin_getBaseline + 50, baseline);
            // cout<<i<<"  Integral=   "<< Integral<<endl;
        }
        if( n_Peak==1 )
        {
            chargeHist->Fill(Integral);
        }
        // cout<<Integral<<endl;
        v_integral.push_back(Integral);
    }



    if ( debug==true )
    {
        TCanvas *c1=new TCanvas("chargeHist","chargeHist",800,600);
        chargeHist->DrawCopy();
    }


    int min=0;
    int binmin=10000000;
    for (int j=0;j<20;j++){
        if (chargeHist->GetBinContent(j+1)<binmin) {
            binmin=chargeHist->GetBinContent(j+1);
            min=j+1;
        }
    }
    min=min*100;
    cout<<"min="<<min<<endl;
    int found=0;
    int min2=0;
    int binmin2=10000000;
    for (int j=20;j<200;j++){
        if (found==0 && chargeHist->GetBinContent(j+1)<binmin) {
            binmin2=chargeHist->GetBinContent(j+1);
            min2=j+1;
            found=1;
        }
    }
    min2=min2*100;
    cout<<"min2="<<min2<<endl;


    TCanvas* can8=new TCanvas("SPE"+(TString) n2str(times_rms),"SPE"+(TString) n2str(times_rms),800,600);
    //chargeHist->Fit("gaus");
    TF1* fun1=NULL;

    double low= v_range[0];
    double high= v_range[1];

    chargeHist->DrawCopy();

    //chargeHist->Fit("gaus","Q");
    //TF1* fun1=chargeHist->GetFunction("gaus");
    //cout<<fun1->GetParameter(1)<<" and "<<fun1->GetParameter(2)<<endl;
    if ( single_peak_fit == false )
    {
        if (low<min) low=min;
        if (high<min || low>min2) cout<<"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"<<endl;
        if (high>min2) high=min2;
    }
    cout<<"low="<<low<<" high="<<high<<endl;
    delete fun1;
    int countspe=0;
    for (int i=0;i<entry;i++){

        Integral = v_integral[i];
        // cout<< "Integral: "<<Integral<<endl;
        // cout<< "high and low:  "<<high<<"   "<<low<<endl;
        if (Integral>low &&Integral<high && v_PeakNum[i]==1 && v_check_NBinToGetBaseline[i]==true)
        {
            isSPE->SetBinContent(i+1,1);
            countspe++;
        }
        //                                                                                                      cout<<Integral<<","<<endl;
    }
    if (plot_check_IsSPE_to_pdf == true)
    {
        const int n_line_to_pdf = 10;
        vector<vector<TH1D*>> v2D_signal_TH1D_toPDF( n_line_to_pdf );
        int n_to_pdf_waves=0;
        int n_c=0;
        for (int i = 0; i < entry ; i++)
        {
            tr->GetEntry(i);
            //注意！！！使用isSPE时，记住它是从1开始的，所以时刻记住要用（i+1）去提取数据
            if( isSPE->GetBinContent(i+1) ==1 )
            {
                waveform->SetBinContent(parsWaves.nDimension, waveform->GetBinContent(parsWaves.nDimension - 3));
                // waveform->SetBinContent(0, waveform->GetBinContent(parsWaves.nDimension-3));
                // cout<<i<<endl;
                waveform->SetXTitle("Time (ns)");
                waveform->SetYTitle("ADC");
                v2D_signal_TH1D_toPDF[n_to_pdf_waves/4].push_back(
                        (TH1D*) waveform->Clone( (TString)n2str(i)+"h_waveform_isSPE="+(TString)n2str( isSPE->GetBinContent(i+1) ) ));
                n_to_pdf_waves++;
                if(n_to_pdf_waves==n_line_to_pdf*4)break;
                if( n_c<15 && plot_selected_result==true )
                {
                    TCanvas *c1=new TCanvas("waves"+(TString)n2str(i),"waves"+(TString)n2str(i),800,600);
                    waveform->DrawCopy();
                    bool whether_NBinToGetBaseline_isReliable=check_NBinsToGetBaseline(waveform, parsWaves.n_bin_getBaseline, threshold_CheckBaseline);
                    cout<< i<<"  Whether reliable?  "<<whether_NBinToGetBaseline_isReliable<<endl;
                    cout << i << "  N_Peak:   " << GetWaveValleyNum(waveform, parsWaves.n_bin_getBaseline, baseline) << endl;
                }
                n_c++;
            }
        }

        if ( useThereshold50 == true )
        {
            name_option.Append("_useThreshold50");
        }
        else
        {
            name_option.Append("NoUseThreshold50");
        }

        name_option.Append("_"+(TString)n2str(sigma_selectSinglePhoton)+"SelectSigma");

        plot_into_pdf(v2D_signal_TH1D_toPDF, parsWaves.name_PdfDir + "chargeHist/" + newname + "_checkSinglePhotonWaves" + name_option + ".pdf");
        for (int z = 0; z < v2D_signal_TH1D_toPDF.size(); z++)
        {
            //释放内存
            vector<TH1D*>().swap(v2D_signal_TH1D_toPDF[z]);
        }

    }

    TString name0=newname;
    name0.Append("_check");
    if( useThereshold50== true )
    {
        name0.Append("_useThreshold50");
    }
    else
    {
        name0.Append("_NoUseThreshold");
    }
    if (debug==true)
    {
        name0.Append("_debug");
    }
    TFile* g=new TFile(parsWaves.name_RootFilePath + name0 + ".root", "recreate");
    //TFile* g=new TFile("SPEimage.root","recreate");
    g->cd();
    cout<<"entry="<<entry<<" spe="<<countspe<<endl;
    isSPE->Write();
    chargeHist->Write();

    double n_1Peak=count_NumOfWaves_InCertainNPeak(v_PeakNum, 1);
    double n_2Peak=count_NumOfWaves_InCertainNPeak(v_PeakNum, 2);
    double n_NoCount=count_NumOfWaves_InCertainNPeak(v_PeakNum, -1);
    double ratio_2PeakTo1Peak=n_2Peak/n_1Peak;
    double ratio_1PeakToEffectiveEvent=n_1Peak/(v_PeakNum.size());
    double ratio_1PeakToAllWaves=n_1Peak/entry_waves*2 ;

    TCanvas* c_chargeHist=new TCanvas("c_chargeHist","c_chargeHist",800,600);
    chargeHist->SetTitle("ChargeHist_SelectRange:"+n2str(low)+"---"+n2str(high)+
                         ",NSelectedWaves:"+n2str(countspe)+",1Peak N:"+n2str(n_1Peak)
                         +",Ratio 2PeakTo1:"+n2str(ratio_2PeakTo1Peak)+",1PeakToAll:"+n2str(ratio_1PeakToEffectiveEvent));
    chargeHist->SetXTitle("Integral");
    chargeHist->SetYTitle("N");
    chargeHist->DrawCopy();
    c_chargeHist->SetLogy();

    // ofstream output1;
    // output1.open(parsWaves.name_PdfDir+"Ratio_txt/"+"Ratio.txt");
    TString str_ToRatioTxt=newname+",";
    str_ToRatioTxt+=n2str(ratio_1PeakToEffectiveEvent)+","+n2str(ratio_2PeakTo1Peak)+","+n2str(ratio_1PeakToAllWaves)
                    +","+n2str(countspe)+","+n2str((double)countspe/(double)entry);
    system("mkdir -p " + parsWaves.name_PdfDir + "Ratio_txt/");
    system("echo '" + str_ToRatioTxt + "'>> " + parsWaves.name_PdfDir + "Ratio_txt/" + "Ratio.txt");
    system("mkdir -p " + parsWaves.name_PdfDir + "Entries_Info/");
    system("echo '" + n2str(entry) + "'>" + parsWaves.name_PdfDir + "Entries_Info/" + newname + ".txt");




    c_chargeHist->SaveAs(parsWaves.name_PdfDir + "chargeHist/" + newname + "_ChargeHist" + name_option + ".png");
    plot_into_pdf(v_2Peak_toPdf , parsWaves.name_PdfDir + "chargeHist/" + newname + "_Check2Peak" + name_option + ".pdf");
    // TCanvas* can1=new TCanvas("c2","c2",800,600);
    // can1->cd();
    // chargeHist->DrawCopy();

    g->Close();
    f->Close();
}



#endif //ROOT_TOOLKIT_GETCUTRANGE_AND_ISSPE_BIGPEAK_HH
