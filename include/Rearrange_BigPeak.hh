//
// Created by luoxj on 11/3/20.
//

#ifndef ROOT_TOOLKIT_REARRANGE_BIGPEAK_HH
#define ROOT_TOOLKIT_REARRANGE_BIGPEAK_HH

#include "TTree.h"
#include "TH1D.h"
#include "TFile.h"
#include "TCanvas.h"
#include <sstream>
#include <string.h>
#include "type_transform.hh"
#include "plot.hh"
#include <vector>
#include "pars_waves.hh"
void Rearrange_BigPeakAndSmallPeak( TString nameFile ){
    using namespace std ;

    pars_waves parsWaves;

    bool debug=false;
    bool plot_into_Canvas=false;
    bool plot_ResultIntoPdf=true;
    if (debug==true)
    {
        plot_ResultIntoPdf=false;
    }

    double n_bin_getBaseline=parsWaves.n_bin_getBaseline;
    double loc_Bin_align=parsWaves.loc_Bin_align;
    // bool debug=false;
//    TString nameFile=parsWaves.name_file_spe;

    ////////////////////读取相应的文件////////////////////////////
    TString name_file_RawData=nameFile+"_RawData.root";
    TString name_output= nameFile + "_rearrange_BigPeak.root";
    TFile* f=new TFile(parsWaves.name_RootFilePath + name_file_RawData, "read");
    //TFile* f=new TFile("new1divide.root","read");
    TTree* t=(TTree*) f->Get("SignalBigPeak");
    TTree* t_SmallPeak=(TTree*) f->Get("SignalSmallPeak");
    // TTree* t=(TTree*) f->Get("waves");
    TH1D* waveform_BigPeak=NULL;
    TH1D* waveform_SmallPeak=NULL;
    TH1D* waveform_AlignedBaseline_BigPeak=NULL;
    TH1D* waveform_AlignedBaseline_SmallPeak=NULL;
    t->SetBranchAddress("SignalBigPeak", &waveform_BigPeak );
    t_SmallPeak->SetBranchAddress("SignalSmallPeak", &waveform_SmallPeak );
    int entries=t->GetEntries();
    //int entries=10;
    //////////////////////////////////////////////////////

    /////////初始化要存储的文件//////////////////////////
    if (debug==true ) name_output= "debug_" + name_output;
    TFile* g=new TFile(parsWaves.name_RootFilePath + name_output, "recreate");
    TTree* str=new TTree("waves_BigPeak","waves_BigPeak");
    TTree* str_SmallPeak=new TTree("waves_SmallPeak","waves_SmallPeak");
    TH1D* waveform_Rearrange_BigPeak=NULL;
    TH1D* waveform_Rearrange_SmallPeak=NULL;
    str->Branch("waves_BigPeak","TH1D",&waveform_Rearrange_BigPeak);
    str_SmallPeak->Branch("waves_SmallPeak","TH1D",&waveform_Rearrange_SmallPeak);
    //////////////////////////////////////

    //TH1D* waveform_AlignedBaseline_BigPeak=new TH1D("waveform_AlignedBaseline_BigPeak","waveform_AlignedBaseline_BigPeak",nBin,0,nBin);
    //TH1D* waveform_Rearrange=new TH1D("waveform_Rearrange","waveform_Rearrange",nBin,0,nBin);
    t->GetEntry(1);
    const double nBin = waveform_BigPeak->GetXaxis() ->GetNbins();
    cout<< "nBin:    :"<<nBin<<endl;
    cout<< "Entries of BigPeak:  "<< entries<<endl;
    cout<< "Entries of SmallPeak:  "<< t_SmallPeak->GetEntries()<<endl;
    TH1D* startposition= new TH1D("startposition","startposition",nBin,0,nBin);
    TH1D* startposition_SmallPeak= new TH1D("startposition_SmallPeak","startposition_SmallPeak",nBin,0,nBin);
    int sum=0;
    int sum_SmallPeak=0;
    int maximum=0;
    int maxpoint=0;
    int bin_JustOverThreshold=0;
    if (debug==true) entries=400;
    int n_WavesAborted=0;

    vector<TH1D*> v1D_TH1D;
    vector<TH1D*> v1D_TH1D_SmallPeak;

    for (int k=0;k<entries;k++){
        t->GetEntry(k);
        t_SmallPeak->GetEntry(k);
        // int n_BinMoveWave=0;
        if(k<2&& debug==true)
        {
            TCanvas *c_B=new TCanvas("c_waves_B"+(TString)n2str(k),"c_waves_B"+(TString)n2str(k),800,600);
            waveform_BigPeak->DrawCopy();
            TCanvas *c_S=new TCanvas("c_waves_S"+(TString)n2str(k),"c_waves_S"+(TString)n2str(k),800,600);
            waveform_SmallPeak->DrawCopy();
        }

        waveform_AlignedBaseline_BigPeak=new TH1D(Form("trans_BigPeak%i",k),"waveform_AlignedBaseline_BigPeak",nBin,0,nBin);
        waveform_AlignedBaseline_SmallPeak=new TH1D(Form("trans_SmallPeak%i",k),"waveform_AlignedBaseline_SmallPeak",nBin,0,nBin);

        ///////把波形的基线平移到0的位置_BigPeak/////////////
        for (int i = 0; i < nBin; ++i) {
            waveform_AlignedBaseline_BigPeak->SetBinContent(i+1,-waveform_BigPeak->GetBinContent(i+1));
            waveform_AlignedBaseline_SmallPeak->SetBinContent(i+1, waveform_SmallPeak->GetBinContent(i+1));

        }
//        sum=0;
//        for (int j=0;j<n_bin_getBaseline;j++){
//            sum+=waveform_BigPeak->GetBinContent(j+1);
//        }
//        sum=sum/n_bin_getBaseline;
//        for (int i=0;i<(nBin-1);i++){
//            //因为电子学的信号是负的，所以这里的sum代表的就是baseline，而且是用baseline去减waveform
//            waveform_AlignedBaseline_BigPeak->SetBinContent(i+1,sum-waveform_BigPeak->GetBinContent(i+1));
//        }
//        waveform_AlignedBaseline_BigPeak->SetBinContent(nBin,sum-waveform_BigPeak->GetBinContent((nBin-1)));


        if (debug == true && k<2)
        {
            TCanvas *c2=new TCanvas("c"+(TString)n2str(k),"c"+(TString)n2str(k),800,600);
            waveform_AlignedBaseline_BigPeak->DrawCopy();
        }
        /////////////////////////////////////////////////////

        ///////把波形的基线平移到0的位置_SmallPeak/////////////
//        sum_SmallPeak=0;
//        for (int j=0;j<n_bin_getBaseline;j++){
//            sum_SmallPeak+=waveform_SmallPeak->GetBinContent(j+1);
//        }
//        sum_SmallPeak=sum_SmallPeak/n_bin_getBaseline;
//        for (int i=0;i<(nBin-1);i++){
//            //因为电子学的信号是负的，所以这里的sum_SmallPeak代表的就是baseline，而且是用baseline去减waveform
//            waveform_AlignedBaseline_SmallPeak->SetBinContent(i+1,sum_SmallPeak-waveform_SmallPeak->GetBinContent(i+1));
//        }
//        waveform_AlignedBaseline_SmallPeak->SetBinContent(nBin,sum_SmallPeak-waveform_SmallPeak->GetBinContent((nBin-1)));


        if (debug == true && k<2)
        {
            TCanvas *c2_SmallPeak=new TCanvas("c_SmallPeak"+(TString)n2str(k),"c_SmallPeak"+(TString)n2str(k),800,600);
            waveform_AlignedBaseline_SmallPeak->DrawCopy();
        }
        ///////////////////////////////////////////////////////


        /////////////////////Rearrange waveform of BigPeak/////////////////////
        //找每个波形的最大值以及最大值对应的bin
        maximum=0;
        maxpoint=0;
        for (int i=0;i<nBin;i++){
            if (waveform_AlignedBaseline_BigPeak->GetBinContent(i+1)>maximum){
                maximum=waveform_AlignedBaseline_BigPeak->GetBinContent(i+1);
                maxpoint=i+1;
            }
        }

        maximum=maximum/6;
        //compare这个参数是为了看波形什么时候过阈值，即过阈的bin在哪个位置
        bin_JustOverThreshold=0;
        for (int i=maxpoint;i>0;i--){
            if (waveform_AlignedBaseline_BigPeak->GetBinContent(i+1)<maximum && bin_JustOverThreshold==0){
                bin_JustOverThreshold=i+1;
            }
        }
        //if (compare==0) compare=512;
        //startposition是为了看trigger的位置的分布
        startposition->Fill(bin_JustOverThreshold);
        //	cout<<"sum="<<sum<<" maxpoint="<<maxpoint<<" compare="<<compare<<" maximum="<<maximum<<endl;
        waveform_Rearrange_BigPeak=new TH1D(Form("arrage%i",k),"waveform_Rearrange_BigPeak",nBin,0,nBin);
        /*for (int s=0;s<nBin;s++) {
                        waveform_Rearrange_BigPeak->SetBinContent(s+1,0);
                }*/
        if (bin_JustOverThreshold<loc_Bin_align && maximum>0){
            for (int j=loc_Bin_align-bin_JustOverThreshold;j<nBin;j++){
                waveform_Rearrange_BigPeak->SetBinContent(j+1,waveform_AlignedBaseline_BigPeak->GetBinContent(bin_JustOverThreshold-loc_Bin_align+j+1));
            }

            for (int j=0;j<loc_Bin_align-bin_JustOverThreshold;j++){
                waveform_Rearrange_BigPeak->SetBinContent(j+1,waveform_AlignedBaseline_BigPeak->GetBinContent(1));
                // n_BinMoveWave++;
            }
        }
        if (bin_JustOverThreshold>=loc_Bin_align && maximum>0){
            for (int j=0;j<nBin-bin_JustOverThreshold+loc_Bin_align;j++){
                waveform_Rearrange_BigPeak->SetBinContent(j+1,waveform_AlignedBaseline_BigPeak->GetBinContent(bin_JustOverThreshold-loc_Bin_align+j+1));
            }
            for (int j=nBin-bin_JustOverThreshold+loc_Bin_align;j<nBin;j++){
                waveform_Rearrange_BigPeak->SetBinContent(j+1,waveform_AlignedBaseline_BigPeak->GetBinContent(nBin));
                // n_BinMoveWave++;
            }
        }
        ////////////////////////////////////////////////////////////////////////////////

        /////////////////////Rearrange waveform of SmallPeak/////////////////////
        //找每个波形的最大值以及最大值对应的bin
        maximum=0;
        maxpoint=0;
        for (int i=0;i<nBin;i++){
            if (waveform_AlignedBaseline_SmallPeak->GetBinContent(i+1)>maximum){
                maximum=waveform_AlignedBaseline_SmallPeak->GetBinContent(i+1);
                maxpoint=i+1;
            }
        }

        maximum=maximum/6;
        //compare这个参数是为了看波形什么时候过阈值，即过阈的bin在哪个位置
        bin_JustOverThreshold=0;
        for (int i=maxpoint;i>0;i--){
            if (waveform_AlignedBaseline_SmallPeak->GetBinContent(i+1)<maximum && bin_JustOverThreshold==0){
                bin_JustOverThreshold=i+1;
            }
        }
        //if (compare==0) compare=512;
        //startposition是为了看trigger的位置的分布
        startposition_SmallPeak->Fill(bin_JustOverThreshold);
        //	cout<<"sum="<<sum<<" maxpoint="<<maxpoint<<" compare="<<compare<<" maximum="<<maximum<<endl;
        waveform_Rearrange_SmallPeak=new TH1D(Form("arrage_SmallPeak%i",k),"waveform_Rearrange_SmallPeak",nBin,0,nBin);
        /*for (int s=0;s<nBin;s++) {
                        waveform_Rearrange_SmallPeak->SetBinContent(s+1,0);
                }*/
        if (bin_JustOverThreshold<loc_Bin_align && maximum>0){
            for (int j=loc_Bin_align-bin_JustOverThreshold;j<nBin;j++){
                waveform_Rearrange_SmallPeak->SetBinContent(j+1,waveform_AlignedBaseline_SmallPeak->GetBinContent(bin_JustOverThreshold-loc_Bin_align+j+1));
            }

            for (int j=0;j<loc_Bin_align-bin_JustOverThreshold;j++){
                waveform_Rearrange_SmallPeak->SetBinContent(j+1,waveform_AlignedBaseline_SmallPeak->GetBinContent(1));
                // n_BinMoveWave++;
            }
        }
        if (bin_JustOverThreshold>=loc_Bin_align && maximum>0){
            for (int j=0;j<nBin-bin_JustOverThreshold+loc_Bin_align;j++){
                waveform_Rearrange_SmallPeak->SetBinContent(j+1,waveform_AlignedBaseline_SmallPeak->GetBinContent(bin_JustOverThreshold-loc_Bin_align+j+1));
            }
            for (int j=nBin-bin_JustOverThreshold+loc_Bin_align;j<nBin;j++){
                waveform_Rearrange_SmallPeak->SetBinContent(j+1,waveform_AlignedBaseline_SmallPeak->GetBinContent(nBin));
                // n_BinMoveWave++;
            }
        }
        ////////////////////////////////////////////////////////////////////////////////




        //如果波形被移动了超过整个entries的一半，那么就不把这个波形保存到rearrange文件中，从而不会用来计算平均波形
        // if ( n_BinMoveWave > parsWaves.nDimension/2 )
        // {
        // 	// cout<< "The Wave has been moved too far so it is aborted."<<endl;
        // 	n_WavesAborted++;
        // 	continue;
        // }

        if (debug==true && k<2)
        {
            TCanvas *c1=new TCanvas("c_waves"+(TString)n2str(k),"c_waves"+(TString)n2str(k),800,600);
            waveform_AlignedBaseline_SmallPeak->DrawCopy();
            // waveform_BigPeak->DrawCopy();
        }
        if ( plot_ResultIntoPdf == true && k<120 )
        {
            v1D_TH1D.push_back( (TH1D*) waveform_Rearrange_BigPeak ->Clone( "waves_rearanged_BigPeak" ));
            v1D_TH1D_SmallPeak.push_back( (TH1D*) waveform_Rearrange_SmallPeak ->Clone( "waves_rearanged_SmallPeak" ));
        }


        if (k==0){
            TCanvas* can1=new TCanvas("c2","c2",800,600);
            can1->cd();
            waveform_Rearrange_BigPeak->Draw();
        }

        g->cd();
        str->Fill();
        str_SmallPeak->Fill();
        delete waveform_AlignedBaseline_BigPeak;
        delete waveform_Rearrange_BigPeak;
        delete waveform_AlignedBaseline_SmallPeak;
        delete waveform_Rearrange_SmallPeak;
        if (k%10000==0) cout<<k<<" finished!"<<endl;
    }
    g->cd();


    if ( plot_ResultIntoPdf == true )
    {
        system("mkdir -p " + parsWaves.name_PdfDir + "rearange_waves/");
        plot_into_pdf(v1D_TH1D, parsWaves.name_PdfDir + "rearange_waves/" + nameFile + "_rearanged_waves_BigPeak" + parsWaves.name_RawWaveSelectOption + ".pdf");
        plot_into_pdf(v1D_TH1D_SmallPeak, parsWaves.name_PdfDir + "rearange_waves/" + nameFile + "_rearanged_waves_SmallPeak" + parsWaves.name_RawWaveSelectOption + ".pdf");
        TLegend* lengend=new TLegend(0.6,0.6,0.9,0.9);
        TCanvas *c1=new TCanvas("c_startposition","c_waves",800,600);
        lengend->AddEntry(startposition,"StartPosition_BigPeak");
        lengend->AddEntry(startposition_SmallPeak,"StartPosition_SmallPeak");
        startposition->DrawCopy();
        startposition_SmallPeak->SetLineColor(3);
        startposition_SmallPeak->DrawCopy("same");
        lengend->Draw();
        c1->SaveAs(parsWaves.name_PdfDir + "rearange_waves/" + nameFile + "_StartPointsHist_BigPeak" + parsWaves.name_RawWaveSelectOption + ".png");
    }

    // cout<< "n_WavesOriginal:    "<<entries<<endl;
    // cout<< "n_WavesAborted:   "<<n_WavesAborted<<endl;
    // cout<< "n_Waves_remained:   "<<str->GetEntries()<<endl;
    startposition->Write();
    startposition_SmallPeak->Write();
    str->Write();
    str_SmallPeak->Write();
    g->Close();
    f->Close();

}

#endif //ROOT_TOOLKIT_REARRANGE_BIGPEAK_HH
