//
// Created by luoxj on 11/12/20.
//

#ifndef ROOT_TOOLKIT_INTEGRAL_DECONVOLUTIONWAVES_HH
#define ROOT_TOOLKIT_INTEGRAL_DECONVOLUTIONWAVES_HH
#include<vector>
#include "TCanvas.h"
#include "TString.h"
#include "TH1D.h"
#include "TFile.h"
#include "TTree.h"
#include <iostream>
#include "pars_waves.hh"
void integral_deconvolutionWaves( TString name_file)
{
    using namespace std;
    pars_waves parsWaves;
    TString newname = name_file;
    bool coutCheck=false;

//    TFile* f_waves = new TFile(parsWaves.name_RootFilePath + "deconvolution_result/" + newname + "_waves_tran_0.root", "read");
    TFile* f_waves = new TFile(parsWaves.name_RootFilePath + "deconvolution_result/"+ newname + "_waves_tran_0.root", "read");
    TTree* tree_waves = (TTree*) f_waves->Get("waves_trans");
    cout<<"Entries :   "<<tree_waves->GetEntries()<<endl;
    TH1D* h_waves = NULL;
    tree_waves->SetBranchAddress("dividedt_moved", &h_waves);
    const int nDimension=parsWaves.nDimension;
    int N = nDimension;
    int m_length = nDimension;
    vector<double> AC(nDimension);
    vector<double> AC2(nDimension);
    double baseline = 0;
    double m_threshold = 0.04;
    int debug = 0;
    vector<double> time;
    double m_window=3;//ns
    vector<double> charge;
    int m_hc=0;

    for(int i=5;i<11;i++)
    {
        tree_waves->GetEntry(i);
        TString name_c="c";
        name_c=name_c+i;
        TCanvas *c1=new TCanvas(name_c,name_c,800,600);
        h_waves->DrawCopy();
    }
//    TH1D* h_wave_clone=new TH1D("h_waves_clone","h_waves_clone",N,0,N);

    for(int k=0;k<tree_waves->GetEntries();k++)
    {
        if(coutCheck)  cout<<k<<"begin   OK!"<<endl;
        tree_waves->GetEntry(k);
        double max=-100000;
        double min=100000;
//        h_waves->GetMinimumAndMaximum(min, max);
        // min=h_waves->GetMinimum();
        // max=h_waves->GetMaximum();

        AC2 = AC;
        for(int j=0;j<nDimension;j++)
        {
            AC[j]=h_waves->GetBinContent(j);
        }  // wave recon

        for (int j = 0; j < nDimension; j++)
        {
            if ( AC[j]>max)
            {
                max=AC[j];
            }
            if ( AC[j]<min )
            {
                min=AC[j];
            }
        }
        int imax = int(max * 1000.);
        int imin = int(min * 1000.);

        std::vector<int> oneHit;
        std::vector<std::vector<int>> hit;
        std::vector<std::vector<int>> singleHit;
        std::vector<int> singleHitPeak;
        std::vector<int> hitPeakCounter;
        std::vector<int> hitWidth;

        if(coutCheck)  cout<<k<<"Initialize OK!"<<endl;
        // rough baseline
        // for (int i = 0; i < 40; i++) baseline += AC[i];
        // baseline /= 40.;
        const int tmpsize = imax - imin + 3;
        vector<int> tmpv(tmpsize);
        for (int i = 0; i < m_length; i++) {
            int tmpidx = int(AC[i] * 1000.);
//      cout<<"iMax and iMin are : "<<i<< "  "<<tmpidx<<"  "<<AC[i]<<"  "<< h_waves->GetBinContent(i)<<endl;

            if(coutCheck&& k==12783)  cout<<"check  "<<tmpidx<<endl;
            if(coutCheck&& k==12783)  cout<<"min  "<<imin <<"   "<<min<<endl;
            if(coutCheck&& k==12783)  cout<<"AC  "<<AC[i]<<endl;
            tmpv.at(tmpidx - imin)++;//project the y value to tmpv, so that to get the baseline.
        }
        if(coutCheck)  cout<<k<<"check"<<endl;
        baseline = max_element(tmpv.begin(), tmpv.end()) - tmpv.begin() + imin;
        baseline /= 1000.;
        for (int i = 0; i < nDimension; i++) AC[i] -= baseline;
        if(coutCheck)  cout<<k<<"rough baseline   OK!"<<endl;

        // rough hits
        oneHit.clear();
        hit.clear();
        int Pass = 0;
        for (int i = 0; i < N; i++) {
            if (Pass == 0 && AC[i] > m_threshold) {
                for (int j = i; j >= 0; j--) {
                    oneHit.insert(oneHit.begin(), j);
                    if (AC[j] <= 0) break;
                }
                Pass = 1;
                continue;
            }
            if (Pass == 1) {
                oneHit.push_back(i);
                if (AC[i] <= 0 || i == N - 1) {
                    if (oneHit.size() > 12) hit.push_back(oneHit);//too narrow pulse will be eliminated.
                    Pass = 0;
                    oneHit.clear();
                }
            }
            if (hit.size() > 990) {
                std::cout << "Too much hits recognized! Move to next waveform." << std::endl;
                break;  // if it really happend, it should print out warning and move to next waveform.
            }
        }
        if(coutCheck)  cout<<k<<"rough hits   OK!"<<endl;

        // accurate baseline
        int flag;
        int counter = 0;
        baseline = 0;
        for (int i = 0; i < N; i++) {
            flag = 1;
            for (int j = 0; j < hit.size(); j++) {
                if (i >= hit.at(j).front() - 9 && i <= hit.at(j).back() + 9) {
                    flag = 0;
                    break;
                }
            }
            if (flag != 0) {
                baseline += AC[i];
                counter++;
            }
        }
        if (counter < 40)
        {
            std::cout << "No enough points for baseline calculation, baseline will set to be the default one." << std::endl;  // make sure to have enough points to calculate baseline
        } else if (counter != 0) {
            baseline /= (counter + 0.);
            for (int i = 0; i < N; i++) AC[i] -= baseline;
        }
        if(coutCheck)  cout<<k<<"accurate baseline   OK!"<<endl;

        // accurate hits
        hit.clear();
        oneHit.clear();  // only when "if (hit.size() > 990) break;" happens, it will be used.
        Pass = 0;
        for (int i = 0; i < N; i++) {
            if (Pass == 0 && AC[i] > m_threshold) {
                for (int j = i; j >= 0; j--) {
                    oneHit.insert(oneHit.begin(), j);
                    if (AC[j] <= 0) break;
                }
                Pass = 1;
                continue;
            }
            if (Pass == 1) {
                oneHit.push_back(i);
                if (AC[i] <= 0 || i == N - 1) {
                    if (oneHit.size() > 12) hit.push_back(oneHit);  // 12 ns width for J19
                    Pass = 0;
                    oneHit.clear();
                }
            }
            if (hit.size() > 990) {
                cout << "Too much hits recognized! Move to next waveform." << std::endl;
                break;  // if it really happend, it should print out warning and move to next waveform.
            }
        }
        if(coutCheck)  cout<<k<<"accurate hits   OK!"<<endl;

        // find singal hit
        singleHit.clear();
        singleHitPeak.clear();
        hitPeakCounter.clear();
        int Rise = 1;
        counter = 0;
        int HitCount = hit.size();
        int HitBTime[1000] = {0};
        int HitETime[1000] = {0};
        hitWidth.clear();
        for (int i = 0; i < hit.size(); i++) {
            Rise = 1;
            int sT = -1;
            int eT;
            hitPeakCounter.push_back(0);
            // record hit starting time and ending time
            HitBTime[i] = hit.at(i).front();
            HitETime[i] = hit.at(i).back();
            for (int j = 0; j < hit.at(i).size(); j++) {
                // find vally
                if (Rise == 0) {
                    if (AC[hit.at(i).at(j)] > AC[hit.at(i).at(j - 1)]) {
                        singleHit.push_back(oneHit);
                        oneHit.clear();
                        counter++;
                        Rise = 1;
                    }
                }
                oneHit.push_back(hit.at(i).at(j));
                // find peak
                if (Rise == 1 && j > 0) {
                    if (AC[hit.at(i).at(j)] < AC[hit.at(i).at(j - 1)]) {
                        singleHitPeak.push_back(hit.at(i).at(j - 1));
                        if (AC[hit.at(i).at(j - 1)] >= m_threshold) hitPeakCounter.at(i)++;
                        Rise = 0;
                    }
                }
                // find overthreshold starting and ending point and their width
                if (AC[hit.at(i).at(j)] >= m_threshold) {
                    eT = j;
                    if (sT == -1) sT = j;
                }
            }
            // finish imcompleted hits, most are half peak at beginning or last of waveform
            if (oneHit.size() != 0) {
                if (Rise == 1) {
                    singleHitPeak.push_back(hit.at(i).back());
                    if (AC[hit.at(i).back()] >= m_threshold) hitPeakCounter.at(i)++;
                }
                singleHit.push_back(oneHit);
                oneHit.clear();
                counter++;
            }
            hitWidth.push_back(eT - sT + 1);
        }
        // exclude fake peaks which are underthreshold
        for (int i = 0; i < singleHitPeak.size(); i++) {
            if (AC[singleHitPeak.at(i)] < m_threshold) {
                singleHitPeak.erase(singleHitPeak.begin() + i);
                i--;
                counter--;
            }
        }
        // peak number test, robust check
        if (counter != singleHitPeak.size()) {
            cout << "Error! Wrong hit peak counting!" << std::endl;
        }
        int tempN = 0;
        for (int i = 0; i < hitPeakCounter.size(); i++) tempN += hitPeakCounter.at(i);
        if (tempN != singleHitPeak.size() || hitPeakCounter.size() != hit.size()) {
            cout << "Error2! Wrong hit peak counting!" << std::endl;
        }
        if(coutCheck)  cout<<k<<"find hits   OK!"<<endl;

        int inteW = m_window;
//      if (HitCount == 0) return true;
        for (int i = 0; i < HitCount; i++) {
            time.push_back(HitBTime[i]);
            int inte_begin = 0, inte_end = 0;
            int singleHit = 0;
            if (HitCount == 1) {
                inte_begin = HitBTime[i] - inteW + 1;
                if (inte_begin < 0) inte_begin = 0;
                inte_end = HitETime[i] + inteW;
                if (inte_end > N) inte_end = N;
                singleHit = 1;
            } else {
                if (i == 0) {
                    inte_begin = HitBTime[i] - inteW + 1;
                    if (inte_begin < 0) inte_begin = 0;
                    int interval = HitBTime[1] - HitETime[0];
                    if (interval > 2 * inteW) {
                        inte_end = HitETime[0] + inteW;
                        singleHit = 1;
                    } else
                        inte_end = int(HitETime[0] + interval / 2.);
                } else if (i == HitCount - 1) {
                    inte_end = HitETime[i] + inteW;
                    if (inte_end > N) inte_end = N;
                    int interval = HitBTime[i] - HitETime[i - 1];
                    if (interval > 2 * inteW) {
                        inte_begin = HitBTime[i] - inteW + 1;
                        singleHit = 1;
                    } else
                        inte_begin = int(HitBTime[i] - interval / 2.);
                } else {
                    int single1 = 0, single2 = 0;
                    int interval = HitBTime[i + 1] - HitETime[i];
                    if (interval > inteW * 2) {
                        inte_end = HitETime[i] + inteW;
                        single1 = 1;
                    } else
                        inte_end = int(HitETime[i] + interval / 2.);
                    interval = HitBTime[i] - HitETime[i - 1];
                    if (interval > 2 * inteW) {
                        inte_begin = HitBTime[i] - inteW + 1;
                        single2 = 1;
                    } else
                        inte_begin = int(HitBTime[i] - interval / 2.);
                    if (single1 == 1 && single2 == 1) singleHit = 1;
                }
            }
            double tempC = 0;
            for (int j = inte_begin; j < inte_end; j++) tempC += AC[j];
            if (m_hc) {  // the hit counting method is opened
                if (singleHit) {
                    if (tempC < 1.55) tempC = 1;  // A simple hit counting, to remove the PMT amplitude fluctuations.
                    // else if(tempC<2.5) tempC = 2;
                    // else if(tempC<3.5) tempC = 3;
                }
            }
            charge.push_back(tempC);

        }
        if(coutCheck)  cout<<k<<"get Charge   OK!"<<endl;

    }
    TH1D* h_Integral = new TH1D("h_Integral","h_Integral",200,0,10);
    for(int i=0;i<charge.size();i++)
    {
        h_Integral->Fill(charge[i]);
    }
    TCanvas *c1=new TCanvas("Integral","Integral",800,600);
    h_Integral->SetXTitle("Integral");
    h_Integral->SetYTitle("N");
    h_Integral->DrawCopy();
    system("mkdir -p " + parsWaves.name_PdfDir + "integral_AfterDecon/");
    c1->SaveAs(parsWaves.name_PdfDir + "integral_AfterDecon/" + newname + "_IntegralAfterDeconvolution.png");
    TFile* outfile = new TFile(parsWaves.name_RootFilePath + newname + "_IntegralAfterDeconvolution.root", "recreate");
    outfile->cd();
    h_Integral->Write();

    for(int i=0;i<10;i++)
    {
        cout<<i<<"   Time:   "<<time.at(i)<<endl;
    }
    f_waves->Close();
}

#endif //ROOT_TOOLKIT_INTEGRAL_DECONVOLUTIONWAVES_HH
