#pragma once
#include "TH2F.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TString.h"
#include <string>
#include <iostream>
#include <fstream>
#include "TTree.h"
#include "TH1D.h"
#include "TH2D.h"
#include "pars_waves.hh"
#include "plot.hh"
#include "vec_op.hh"

//Attention!! baseline is above the signal.
bool check_whether_real_signal( const vector<double> waveform_substract_baseline )
{
    double baseline = 0;
    double max = vec_op::max(waveform_substract_baseline);
    double min = vec_op::min(waveform_substract_baseline);
    if ( abs(min - baseline)*0.5 > max-baseline )
    {
        return true;
    }
    else
    {
        return false;
    }
}

void ReadData_zhongshan_vectorVersion( TString inputFilePath )
{
    pars_waves parsWaves;
    TString name_currentScript_outputPDF_Dir="raw_wave";
    system("mkdir -p "+parsWaves.name_PdfDir+"/"+name_currentScript_outputPDF_Dir);
    bool debug_plot_to_pdf = true;
    bool debug_draw = false;
    bool debug_draw2D = true;
    using namespace std;
//    string inputFilePath = string("/pmtfs/disk02/container_data/container4/test/FADC_Run/") + string(daq) + string("/") +
//            string(gcuid)+string("_")+string(gcusubid) +
//            string("_") +string(date)+string("_")+ to_string(frame_length) + string("_")+string(daq);
    int global_range_id=0;
    ifstream inputFile;
    inputFile.open(inputFilePath,ios::in);

    unsigned short oneSamplePoint;  // 10-bit sampling value takes 2 Bytes to store.
    int lengthOfEachWave = 1008;
    int fcount=0; //frame count
    vector<int> waveframe;
    vector<double> waveframe_substract_baseline;
    vector<double> waveframe_BigPeakSignal;
    vector<double> waceframe_BigPeakNoise;
    vector<double> waveframe_SmallPeakSignal;
    vector<double> waveframe_SmallPeakNoise;

    double baseline=0;
    double baseline_tail=0;
    int n_BigPeakSignal=0;
    int n_SmallPeakSignal=0;
    int n_BigPeakNoise=0;
    int n_SmallPeakNoise=0;

    TString outputFilePath = parsWaves.name_RootFilePath;
    outputFilePath+=parsWaves.name_file+"_RawData.root";
    TFile * outputAnalysedFile = new TFile(outputFilePath,"RECREATE");
    TTree * tree_rawWaveform = new TTree( "waves", "raw_data");
//    gROOT->ProcessLine("#include <vector>");
    tree_rawWaveform->Branch("raw_wave", &waveframe);
    tree_rawWaveform->Branch("raw_wave_sub_baseline", &waveframe_substract_baseline);
    tree_rawWaveform->Branch("SignalBigPeak", &waveframe_BigPeakSignal);
    tree_rawWaveform->Branch("SignalSmallPeak",&waveframe_SmallPeakSignal);
    tree_rawWaveform->Branch("NoiseBigPeak", &waceframe_BigPeakNoise);
    tree_rawWaveform->Branch("NoiseSmallPeak",&waveframe_SmallPeakNoise);

    vector<TH1D*> v_rawWave_to_pdf;
    vector<vector<double>> v2D_BigPeak_to_pdf;
    vector<vector<double>> v2D_Noise_to_pdf;
    TH2D* h2D_raw = new TH2D("h2D_raw","h2D_raw",lengthOfEachWave,0, lengthOfEachWave,11800-11000,11000,11800);
    TH2D* h2D_raw_sub_baseline= new TH2D( "h2D_raw_sub_baseline", "h2D_raw_sub_baseline", lengthOfEachWave, 0, lengthOfEachWave, 1000, -800, 200 );
    TH2D* h2D_BigPeak_sub_baseline= new TH2D( "h2D_BigPeak_sub_baseline", "h2D_BigPeak_sub_baseline", lengthOfEachWave, 0, lengthOfEachWave, 1000, -800, 200 );
    TH2D* h2D_SmallPeak_sub_baseline= new TH2D( "h2D_SmallPeak_sub_baseline", "h2D_SmallPeak_sub_baseline", lengthOfEachWave, 0, lengthOfEachWave, 1000, -800, 200 );
    TH2D* h2D_BigPeakNoise_sub_baseline= new TH2D("h2D_BigPeakNoise_sub_baseline", "h2D_BigPeakNoise_sub_baseline", lengthOfEachWave, 0, lengthOfEachWave, 1000, -800, 200 );
    TH2D* h2D_SmallPeakNoise_sub_baseline= new TH2D("h2D_SmallPeakNoise_sub_baseline", "h2D_SmallPeakNoise_sub_baseline", lengthOfEachWave, 0, lengthOfEachWave, 1000, -800, 200 );
    bool whether_Signal_BigPeak= false;
    bool whether_Signal_SmallPeak= false;

//    for (int jj = 0; jj < 100000; jj++)
    while (!inputFile.eof()) {
        if (!inputFile.eof()) { fcount++; }
        waveframe.clear();
        //        cout<<"Current File Num:"<<jj<<endl;
        /**********the header of frame*************/
        for (int ii = 0; ii < 8; ii++) {
            inputFile.read(reinterpret_cast<char *>(&oneSamplePoint),
                           2);     // Using reinterpret_cast to convert binary data.
            oneSamplePoint = __builtin_bswap16(oneSamplePoint);
            if (ii == 1)global_range_id = oneSamplePoint % 2;
            if (ii == 1)global_range_id = oneSamplePoint & 0x1;
            //	global_range_id=0;
//            cout<<"ggggg"<<global_range_id<<endl;
        }
        /**********the data of frame*************/
        for (int ii = 0; ii < 1008; ii++)
//        for (int ii = 0; ii < lengthOfEachWave; ii++)
        {
            inputFile.read(reinterpret_cast<char *>(&oneSamplePoint),
                           2);     // Using reinterpret_cast to convert binary data.
            oneSamplePoint = __builtin_bswap16(oneSamplePoint);
            waveframe.push_back(oneSamplePoint);
            //	cout<<oneSamplePoint<<endl;
        }
        /**********the tail of frame*************/
        for (int ii = 0; ii < 8; ii++) {
            inputFile.read(reinterpret_cast<char *>(&oneSamplePoint),
                           2);     // Using reinterpret_cast to convert binary data.
            oneSamplePoint = __builtin_bswap16(oneSamplePoint);
        }


        baseline = 0;
        for (int z = 0; z < parsWaves.n_bin_getBaseline; ++z) {
            baseline += waveframe.at(z);
        }
        baseline /= parsWaves.n_bin_getBaseline;

        baseline_tail = 0;
        for (int z = lengthOfEachWave - parsWaves.n_bin_getBaseline; z < lengthOfEachWave; ++z) {
            baseline_tail += waveframe.at(z);
        }
        baseline_tail /= parsWaves.n_bin_getBaseline;
        if (baseline < baseline_tail) baseline = baseline_tail;


        for (int k = 0; k < lengthOfEachWave; ++k) {
            waveframe_substract_baseline.push_back(waveframe.at(k) - baseline);
        }

        if (global_range_id) {
            whether_Signal_BigPeak=check_whether_real_signal(waveframe_substract_baseline);
            if (whether_Signal_BigPeak) {
                for (int z = 0; z < lengthOfEachWave; ++z) {
                    waveframe_BigPeakSignal.push_back(waveframe_substract_baseline.at(z));
                    h2D_BigPeak_sub_baseline->Fill(z + 1, waveframe_BigPeakSignal.at(z));
                }
                n_BigPeakSignal++;
//                waveframe_BigPeakSignal.assign(waveframe_substract_baseline.begin(), waveframe_substract_baseline.end() );
            } else {
                for (int z = 0; z < lengthOfEachWave; ++z) {
                    waceframe_BigPeakNoise.push_back(waveframe_substract_baseline.at(z));
                    h2D_SmallPeakNoise_sub_baseline->Fill(z + 1, waceframe_BigPeakNoise.at(z));
                }
                n_BigPeakNoise++;
//                waceframe_BigPeakNoise.assign(waveframe_substract_baseline.begin(), waveframe_substract_baseline.end());
            }
        }
        else
        {
            whether_Signal_SmallPeak=check_whether_real_signal(waveframe_substract_baseline);
            if (whether_Signal_SmallPeak) {
                for (int z = 0; z < lengthOfEachWave; ++z) {
                    waveframe_SmallPeakSignal.push_back(waveframe_substract_baseline.at(z));
                    h2D_SmallPeak_sub_baseline->Fill(z + 1, waveframe_SmallPeakSignal.at(z));
                }
                n_SmallPeakSignal++;
//                waveframe_SmallPeakSignal.assign(waveframe_substract_baseline.begin(), waveframe_substract_baseline.end() );
            } else {
                for (int z = 0; z < lengthOfEachWave; ++z) {
                    waveframe_SmallPeakNoise.push_back(waveframe_substract_baseline.at(z));
                    h2D_BigPeakNoise_sub_baseline->Fill(z + 1, waveframe_SmallPeakNoise.at(z));
                }
                n_BigPeakNoise++;
//                waceframe_SmallPeakNoise.assign(waveframe_substract_baseline.begin(), waveframe_substract_baseline.end());
            }
        }

        if (debug_draw2D == true) {
            for (int k = 0; k < lengthOfEachWave; ++k) {
                h2D_raw->Fill(k + 1, waveframe.at(k));
                h2D_raw_sub_baseline->Fill(k + 1, waveframe_substract_baseline.at(k));
            }
        }

//        if(!global_range_id)continue;
        if (debug_draw == true && fcount < 5) {
            TH1D *h_wave = new TH1D("h_raw_wave", "h_raw_wave", lengthOfEachWave, 0, lengthOfEachWave);
            for (int k = 0; k < lengthOfEachWave; ++k) {
                h_wave->SetBinContent(k + 1, waveframe.at(k));
            }
            TString name_c = "c";
            TCanvas *c = new TCanvas(name_c + fcount, name_c + fcount, 800, 600);
            h_wave->DrawCopy();
//            h_wave->Delete();
            if (fcount == 4) break;
        }
        if (debug_plot_to_pdf == true && fcount < 100) {
            TString name_h_pdf = "h_rawWave";
            TH1D *h_wave_toPDF = new TH1D(name_h_pdf + "_pdf" + fcount, name_h_pdf + "_pdf" + fcount, lengthOfEachWave,
                                          0, lengthOfEachWave);
            for (int k = 0; k < lengthOfEachWave; ++k) {
                h_wave_toPDF->SetBinContent(k + 1, waveframe.at(k));
            }
            h_wave_toPDF->SetStats(kFALSE);
            v_rawWave_to_pdf.push_back((TH1D *) h_wave_toPDF->Clone(name_h_pdf + fcount));
            h_wave_toPDF->Delete();
        }
        if (global_range_id)
        {
            if (whether_Signal_BigPeak)
            {
                if (debug_plot_to_pdf == true && n_BigPeakSignal < 100 )v2D_BigPeak_to_pdf.push_back(waveframe_BigPeakSignal);
            } else
            {
                if (debug_plot_to_pdf == true && n_BigPeakNoise < 100 )v2D_Noise_to_pdf.push_back(waceframe_BigPeakNoise);
            }
        }

        tree_rawWaveform->Fill();
        waveframe.clear();
        waveframe_substract_baseline.clear();
        waveframe_BigPeakSignal.clear();
        waceframe_BigPeakNoise.clear();
        waveframe_SmallPeakSignal.clear();
        waveframe_SmallPeakNoise.clear();
    }
    if ( debug_draw2D == true )
    {
        TCanvas* c_TH2D = new TCanvas("c_h2D","c_h2D",800,600);
        h2D_raw->SetStats(kFALSE);
        h2D_raw->SetXTitle("Time [ns]");
        h2D_raw->SetYTitle("ADC");
        c_TH2D->SetLogz();
        h2D_raw->DrawCopy("colz");

        TCanvas* c_TH2D_sub_baseline = new TCanvas("c_h2D_sub_baseline","c_h2D_sub_baseline",800,600);
        h2D_raw_sub_baseline->SetStats(kFALSE);
        h2D_raw_sub_baseline->SetXTitle("Time [ns]");
        h2D_raw_sub_baseline->SetYTitle("ADC");
        c_TH2D_sub_baseline->SetLogz();
        h2D_raw_sub_baseline->DrawCopy("colz");

        TCanvas* c_TH2D_Noise_BigPeaksub_baseline = new TCanvas("c_h2D_Noise_BigPeak_sub_baseline","c_h2D_Noise_BigPeak_sub_baseline",800,600);
        h2D_BigPeakNoise_sub_baseline->SetStats(kFALSE);
        h2D_BigPeakNoise_sub_baseline->SetXTitle("Time [ns]");
        h2D_BigPeakNoise_sub_baseline->SetYTitle("ADC");
        c_TH2D_Noise_BigPeaksub_baseline->SetLogz();
        h2D_BigPeakNoise_sub_baseline->DrawCopy("colz");

        TCanvas* c_TH2D_BigPeaksub_baseline = new TCanvas("c_h2D_BigPeak_sub_baseline","c_h2D_BigPeak_sub_baseline",800,600);
        h2D_BigPeak_sub_baseline->SetStats(kFALSE);
        h2D_BigPeak_sub_baseline->SetXTitle("Time [ns]");
        h2D_BigPeak_sub_baseline->SetYTitle("ADC");
        c_TH2D_BigPeaksub_baseline->SetLogz();
        h2D_BigPeak_sub_baseline->DrawCopy("colz");

        TCanvas* c_TH2D_Noise_SmallPeaksub_baseline = new TCanvas("c_h2D_Noise_SmallPeak_sub_baseline","c_h2D_Noise_SmallPeak_sub_baseline",800,600);
        h2D_SmallPeakNoise_sub_baseline->SetStats(kFALSE);
        h2D_SmallPeakNoise_sub_baseline->SetXTitle("Time [ns]");
        h2D_SmallPeakNoise_sub_baseline->SetYTitle("ADC");
        c_TH2D_Noise_SmallPeaksub_baseline->SetLogz();
        h2D_SmallPeakNoise_sub_baseline->DrawCopy("colz");

        TCanvas* c_TH2D_SmallPeaksub_baseline = new TCanvas("c_h2D_SmallPeak_sub_baseline","c_h2D_SmallPeak_sub_baseline",800,600);
        h2D_SmallPeak_sub_baseline->SetStats(kFALSE);
        h2D_SmallPeak_sub_baseline->SetXTitle("Time [ns]");
        h2D_SmallPeak_sub_baseline->SetYTitle("ADC");
        c_TH2D_SmallPeaksub_baseline->SetLogz();
        h2D_SmallPeak_sub_baseline->DrawCopy("colz");

        c_TH2D->SaveAs(parsWaves.name_PdfDir+name_currentScript_outputPDF_Dir+"/TH2D_c_raw_waves.png");
        c_TH2D_sub_baseline->SaveAs(parsWaves.name_PdfDir+name_currentScript_outputPDF_Dir+"/TH2D_c_raw_waves_sub_baseline.png");
        c_TH2D_BigPeaksub_baseline->SaveAs(parsWaves.name_PdfDir+name_currentScript_outputPDF_Dir+"/TH2D_c_BigPeak.png");
        c_TH2D_Noise_BigPeaksub_baseline->SaveAs(parsWaves.name_PdfDir+name_currentScript_outputPDF_Dir+"/TH2D_c_BigPeak_Noise.png");
        c_TH2D_SmallPeaksub_baseline->SaveAs(parsWaves.name_PdfDir+name_currentScript_outputPDF_Dir+"/TH2D_c_SmallPeak.png");
        c_TH2D_Noise_SmallPeaksub_baseline->SaveAs(parsWaves.name_PdfDir+name_currentScript_outputPDF_Dir+"/TH2D_c_SmallPeak_Noise.png");
    }

    cout<<"the total frame number is:"<<fcount<<endl;
    outputAnalysedFile->cd();
    tree_rawWaveform->Write();
    if (debug_plot_to_pdf)
    {
        plot_into_pdf(v_rawWave_to_pdf, parsWaves.name_PdfDir+name_currentScript_outputPDF_Dir+"/"+parsWaves.name_file+"_raw_wave.pdf");
        plot_into_pdf(v2D_BigPeak_to_pdf,"SignalBigPeak", parsWaves.nDimension, parsWaves.name_PdfDir+name_currentScript_outputPDF_Dir+"/"+parsWaves.name_file+"BigPeakSignal.pdf");
        plot_into_pdf(v2D_Noise_to_pdf,"Noise_BigPeak",parsWaves.nDimension, parsWaves.name_PdfDir+name_currentScript_outputPDF_Dir+"/"+parsWaves.name_file+"BigPeakNoise.pdf");
    }

    if (debug_draw2D== true)
    {
        h2D_raw->Write();
        h2D_raw_sub_baseline->Write();
    }


    outputAnalysedFile->Close();
}
