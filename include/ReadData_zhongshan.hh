//
// Created by luoxj on 11/3/20.
//

#ifndef ROOT_TOOLKIT_READDATA_ZHONGSHAN_HH
#define ROOT_TOOLKIT_READDATA_ZHONGSHAN_HH
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
bool check_whether_real_signal( TH1D* waveform_substract_baseline ) {
    double baseline = 0;
    double max = waveform_substract_baseline->GetMaximum();
    double min = waveform_substract_baseline->GetMinimum();
    if (abs(min - baseline) * 0.5 > max - baseline) {
        return true;
    } else {
        return false;
    }
}

void draw_TH2D( TH2D* th2D, TString name_save)
{
    TString name_c= th2D->GetTitle();
    name_c+="_c";
    TCanvas* c = new TCanvas( name_c ,name_c,800,600);
    th2D->SetStats(kFALSE);
    th2D->SetXTitle("Time [ns]");
    th2D->SetYTitle("ADC");
    c->SetLogz();
    th2D->DrawCopy("colz");
    c->SaveAs(name_save);
}


void ReadData_zhongshan( TString inputFilePath ) {
    pars_waves parsWaves;
    TString name_currentScript_outputPDF_Dir = "raw_wave";
    system("mkdir -p " + parsWaves.name_PdfDir + "/" + name_currentScript_outputPDF_Dir);
    bool debug_plot_to_pdf = true;
    bool debug_draw2D = true;
    using namespace std;
//    string inputFilePath = string("/pmtfs/disk02/container_data/container4/test/FADC_Run/") + string(daq) + string("/") +
//            string(gcuid)+string("_")+string(gcusubid) +
//            string("_") +string(date)+string("_")+ to_string(frame_length) + string("_")+string(daq);
    int global_range_id = 0;
    ifstream inputFile;
    inputFile.open(inputFilePath, ios::in);

    unsigned short oneSamplePoint;  // 10-bit sampling value takes 2 Bytes to store.
    int lengthOfEachWave = parsWaves.nDimension;
    int fcount = 0; //frame count

//    TH1D* waveframe = NULL;
//    TH1D* waveframe_substract_baseline=NULL ;
//    TH1D* waveframe_BigPeakSignal =NULL;
//    TH1D* waceframe_BigPeakNoise  =NULL;
//    TH1D* waveframe_SmallPeakSignal=NULL;
//    TH1D* waveframe_SmallPeakNoise=NULL;

        TH1D* waveframe = new TH1D(Form("raw_wave%i",fcount),"raw_waves",lengthOfEachWave,0,lengthOfEachWave);
        TH1D* waveframe_substract_baseline= new TH1D(Form("raw_wave_sub_baseline%i",fcount),"raw_waves_sub_baseline",lengthOfEachWave,0,lengthOfEachWave);
        TH1D* waveframe_BigPeakSignal= new TH1D(Form("BigPeakSignal%i",fcount),"BigPeakSignal",lengthOfEachWave,0,lengthOfEachWave);
        TH1D* waceframe_BigPeakNoise= new TH1D(Form("BigPeakNoise%i",fcount),"BigPeakNoise",lengthOfEachWave,0,lengthOfEachWave);
        TH1D* waveframe_SmallPeakSignal= new TH1D(Form("SmallPeakSignal%i",fcount),"SmallPeakSignal",lengthOfEachWave,0,lengthOfEachWave);
        TH1D* waveframe_SmallPeakNoise= new TH1D(Form("SmallPeakNoise%i",fcount),"SmallPeakNoise",lengthOfEachWave,0,lengthOfEachWave);

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
    TTree* tree_signalBigPeak = new TTree("SignalBigPeak", "SignalBigPeak");
    TTree* tree_noise = new TTree("Noise_BigPeak","Noise_BigPeak");
    TTree* tree_signalSmallPeak = new TTree("SignalSmallPeak", "SignalSmallPeak");

//    tree_rawWaveform->Branch("raw_wave","TH1D", &waveframe);
    tree_rawWaveform->Branch("raw_wave_sub_baseline","TH1D", &waveframe_substract_baseline);
    tree_signalBigPeak->Branch("SignalBigPeak","TH1D", &waveframe_BigPeakSignal);
    tree_signalSmallPeak->Branch("SignalSmallPeak","TH1D",&waveframe_SmallPeakSignal);
    tree_noise->Branch("NoiseBigPeak","TH1D", &waceframe_BigPeakNoise);
//    tree_rawWaveform->Branch("NoiseSmallPeak","TH1D",&waveframe_SmallPeakNoise);

    vector<TH1D*> v_rawWave_to_pdf;
    vector<TH1D*> v2D_BigPeak_to_pdf;
    vector<TH1D*> v2D_Noise_to_pdf;



    TH2D* h2D_raw = new TH2D("h2D_raw","h2D_raw",lengthOfEachWave,0, lengthOfEachWave,11800-11000,11000,11800);
    TH2D* h2D_raw_sub_baseline= new TH2D( "h2D_raw_sub_baseline", "h2D_raw_sub_baseline", lengthOfEachWave, 0, lengthOfEachWave, 1000, -800, 200 );
    TH2D* h2D_BigPeak_sub_baseline= new TH2D( "h2D_BigPeak_sub_baseline", "h2D_BigPeak_sub_baseline", lengthOfEachWave, 0, lengthOfEachWave, 1000, -800, 200 );
    TH2D* h2D_SmallPeak_sub_baseline= new TH2D( "h2D_SmallPeak_sub_baseline", "h2D_SmallPeak_sub_baseline", lengthOfEachWave, 0, lengthOfEachWave, 1000, -800, 200 );
    TH2D* h2D_BigPeakNoise_sub_baseline= new TH2D("h2D_BigPeakNoise_sub_baseline", "h2D_BigPeakNoise_sub_baseline", lengthOfEachWave, 0, lengthOfEachWave, 1000, -800, 200 );
    TH2D* h2D_SmallPeakNoise_sub_baseline= new TH2D("h2D_SmallPeakNoise_sub_baseline", "h2D_SmallPeakNoise_sub_baseline", lengthOfEachWave, 0, lengthOfEachWave, 1000, -800, 200 );

    bool whether_Signal_BigPeak= false;
    bool whether_Signal_SmallPeak= false;


    while (!inputFile.eof())
//    for (int jj = 0; jj < 1000; jj++)
    {
        fcount++;

//        waveframe = new TH1D(Form("raw_wave%i",fcount),"raw_waves",lengthOfEachWave,0,lengthOfEachWave);
//        waveframe_substract_baseline= new TH1D(Form("raw_wave_sub_baseline%i",fcount),"raw_waves_sub_baseline",lengthOfEachWave,0,lengthOfEachWave);
//        waveframe_BigPeakSignal= new TH1D(Form("BigPeakSignal%i",fcount),"BigPeakSignal",lengthOfEachWave,0,lengthOfEachWave);
//        waceframe_BigPeakNoise= new TH1D(Form("BigPeakNoise%i",fcount),"BigPeakNoise",lengthOfEachWave,0,lengthOfEachWave);
//        waveframe_SmallPeakSignal= new TH1D(Form("SmallPeakSignal%i",fcount),"SmallPeakSignal",lengthOfEachWave,0,lengthOfEachWave);
//        waveframe_SmallPeakNoise= new TH1D(Form("SmallPeakNoise%i",fcount),"SmallPeakNoise",lengthOfEachWave,0,lengthOfEachWave);


        /**********the header of frame*************/
        for (int ii = 0; ii < 8; ii++) {
            inputFile.read(reinterpret_cast<char *>(&oneSamplePoint),
                           2);     // Using reinterpret_cast to convert binary data.
            oneSamplePoint = __builtin_bswap16(oneSamplePoint);
            if (ii == 1)global_range_id = oneSamplePoint % 2;
            if (ii == 1)global_range_id = oneSamplePoint & 0x1;
        }
        /**********the data of frame*************/
        for (int ii = 0; ii < lengthOfEachWave; ii++)
//        for (int ii = 0; ii < lengthOfEachWave; ii++)
        {
            inputFile.read(reinterpret_cast<char *>(&oneSamplePoint),
                           2);     // Using reinterpret_cast to convert binary data.
            oneSamplePoint = __builtin_bswap16(oneSamplePoint);
            waveframe->SetBinContent(ii+1,oneSamplePoint);
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
            baseline += waveframe->GetBinContent(z+1);
        }
        baseline /= parsWaves.n_bin_getBaseline;

        baseline_tail = 0;
        for (int z = lengthOfEachWave - parsWaves.n_bin_getBaseline; z < lengthOfEachWave; ++z) {
            baseline_tail += waveframe->GetBinContent(z+1);
        }
        baseline_tail /= parsWaves.n_bin_getBaseline;
        if (baseline < baseline_tail) baseline = baseline_tail;

        for (int k = 0; k < lengthOfEachWave; ++k) {
            waveframe_substract_baseline->SetBinContent(k+1, waveframe->GetBinContent(k+1) - baseline);
        }


        if (global_range_id) {
            whether_Signal_BigPeak=check_whether_real_signal(waveframe_substract_baseline);
            if (whether_Signal_BigPeak) {
                waveframe_BigPeakSignal = (TH1D*)waveframe_substract_baseline->Clone("BigPeakSignal");
                for (int z = 0; z < lengthOfEachWave; ++z) {
                    h2D_BigPeak_sub_baseline->Fill(z + 1, waveframe_BigPeakSignal->GetBinContent(z+1));
                }
                n_BigPeakSignal++;
                tree_signalBigPeak->Fill();
            } else {
                waceframe_BigPeakNoise= (TH1D*) waveframe_substract_baseline->Clone("BigPeakNoise");
                for (int z = 0; z < lengthOfEachWave; ++z) {
                    h2D_SmallPeakNoise_sub_baseline->Fill(z + 1, waceframe_BigPeakNoise->GetBinContent(z+1));
                }
                n_BigPeakNoise++;
                tree_noise->Fill();
            }
        }
        else
        {
            whether_Signal_SmallPeak=check_whether_real_signal(waveframe_substract_baseline);
            if (whether_Signal_SmallPeak) {
                waveframe_SmallPeakSignal = (TH1D*)waveframe_substract_baseline->Clone("SmallPeakSignal");
                for (int z = 0; z < lengthOfEachWave; ++z) {
                    h2D_SmallPeak_sub_baseline->Fill(z + 1, waveframe_SmallPeakSignal->GetBinContent(z+1));
                }
                n_SmallPeakSignal++;
                tree_signalSmallPeak->Fill();
            } else {
                waveframe_SmallPeakNoise = (TH1D*) waveframe_substract_baseline->Clone("SmallPeakNoise");
                for (int z = 0; z < lengthOfEachWave; ++z) {
                    h2D_BigPeakNoise_sub_baseline->Fill(z + 1, waveframe_SmallPeakNoise->GetBinContent(z+1));
                }
                n_BigPeakNoise++;
            }
        }

        if (debug_draw2D == true) {
            for (int k = 0; k < lengthOfEachWave; ++k) {
                h2D_raw->Fill(k + 1, waveframe->GetBinContent(k));
                h2D_raw_sub_baseline->Fill(k + 1, waveframe_substract_baseline->GetBinContent(k+1));
            }
        }


        if (debug_plot_to_pdf == true && fcount < 80) {
            TString name_h_pdf = "h_rawWave";
            v_rawWave_to_pdf.push_back((TH1D *) waveframe->Clone(name_h_pdf + fcount));
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
//        cout<<"checking!!!!!!!!!"<<endl;
        if (fcount%10000==0)cout<<fcount<<endl;
        tree_rawWaveform->Fill();
//        waveframe->Delete();
//        waveframe_substract_baseline->Delete();
//        waveframe_BigPeakSignal->Delete();
//        waceframe_BigPeakNoise->Delete();
//        waveframe_SmallPeakSignal->Delete();
//        waceframe_BigPeakNoise->Delete();
    }

    if ( debug_draw2D == true )
    {
        draw_TH2D(h2D_raw, parsWaves.name_PdfDir+name_currentScript_outputPDF_Dir+"/TH2D_c_raw_waves.png");
        draw_TH2D(h2D_raw_sub_baseline,parsWaves.name_PdfDir+name_currentScript_outputPDF_Dir+"/TH2D_c_raw_waves_sub_baseline.png" );
        draw_TH2D(h2D_BigPeak_sub_baseline, parsWaves.name_PdfDir+name_currentScript_outputPDF_Dir+"/TH2D_c_BigPeak.png");
        draw_TH2D(h2D_BigPeakNoise_sub_baseline, parsWaves.name_PdfDir+name_currentScript_outputPDF_Dir+"/TH2D_c_BigPeak_Noise.png");
        draw_TH2D(h2D_SmallPeakNoise_sub_baseline, parsWaves.name_PdfDir+name_currentScript_outputPDF_Dir+"/TH2D_c_SmallPeak.png");
        draw_TH2D(h2D_SmallPeak_sub_baseline, parsWaves.name_PdfDir+name_currentScript_outputPDF_Dir+"/TH2D_c_SmallPeak_Noise.png");
    }

    cout<<"the total frame number is:"<<fcount<<endl;
    outputAnalysedFile->cd();
    tree_rawWaveform->Write();
    tree_signalSmallPeak->Write();
    tree_signalBigPeak->Write();
    tree_noise->Write();
    if (debug_plot_to_pdf)
    {
        plot_into_pdf(v_rawWave_to_pdf, parsWaves.name_PdfDir+name_currentScript_outputPDF_Dir+"/"+parsWaves.name_file+"_raw_wave.pdf");
        plot_into_pdf(v2D_BigPeak_to_pdf, parsWaves.name_PdfDir+name_currentScript_outputPDF_Dir+"/"+parsWaves.name_file+"BigPeakSignal.pdf");
        plot_into_pdf(v2D_Noise_to_pdf,  parsWaves.name_PdfDir+name_currentScript_outputPDF_Dir+"/"+parsWaves.name_file+"BigPeakNoise.pdf");
    }

    if (debug_draw2D== true)
    {
        h2D_raw->Write();
        h2D_raw_sub_baseline->Write();
    }


    outputAnalysedFile->Close();
}





#endif //ROOT_TOOLKIT_READDATA_ZHONGSHAN_HH
