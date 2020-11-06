//
// Created by luoxj on 11/3/20.
//

#ifndef ROOT_TOOLKIT_GETWAVEPEAKNUM_HH
#define ROOT_TOOLKIT_GETWAVEPEAKNUM_HH
#include "TH1D.h"
#include "math.h"
#include "iostream"
// int GetWavePeakNum(const TH1D* waveform,
//                    const int n_bin_GetBaseline,
//                    const double baseline)
// {
//     double times_rms=6;
//     double rms =0 ;
//    	for (int j=0;j<n_bin_GetBaseline;j++)
//     {
// 		rms+=(waveform->GetBinContent(j+1)-baseline)*(waveform->GetBinContent(j+1)-baseline);
// 	}
// 	rms=sqrt((double)rms/n_bin_GetBaseline);

//     bool flag_OverThreshold=false;
//     int n_peak =0 ;
//     for (int i = 0; i < waveform->GetNbinsX(); i++)
//     {
//         if ( waveform->GetBinContent(i) >= baseline+rms*times_rms && flag_OverThreshold==false)
//         {
//             flag_OverThreshold=true;
//             n_peak++;
//         }
//          else if (waveform->GetBinContent(i) <= baseline+rms*times_rms && flag_OverThreshold==true )
//          {
//             flag_OverThreshold=false;
//          }
//     }
//     return n_peak;

// }

int GetWaveValleyNum(const TH1D* waveform,
                     const int n_bin_GetBaseline,
                     const double baseline)
{
    double times_rms=6;
    double rms =0 ;
    for (int j=0;j<n_bin_GetBaseline;j++)
    {
        rms+=(waveform->GetBinContent(j+1)-baseline)*(waveform->GetBinContent(j+1)-baseline);
    }
    rms=sqrt((double)rms/n_bin_GetBaseline);
    // cout<<"baseline-6rms:   "<<baseline-times_rms*rms<<endl;
    // cout<< "baseline:   "<<baseline<<endl;
    // cout<< "rms:   "<<rms<<endl;

    bool flag_UnderThreshold=false;
    int n_valley =0 ;
    for (int i = 0; i < waveform->GetNbinsX(); i++)
    {
        if ( waveform->GetBinContent(i) < baseline-rms*times_rms && flag_UnderThreshold==false)
        {
            flag_UnderThreshold=true;
            n_valley++;
        }
        else if (waveform->GetBinContent(i) > baseline-rms*times_rms && flag_UnderThreshold==true )
        {
            flag_UnderThreshold=false;
        }
    }

    return n_valley;

}
#endif //ROOT_TOOLKIT_GETWAVEPEAKNUM_HH
