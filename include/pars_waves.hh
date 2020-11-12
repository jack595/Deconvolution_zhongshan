#pragma once
#include "TString.h"
#include "iostream"
using namespace std;
struct pars_waves
{

    const static int n_bin_getBaseline=45;
    const static int loc_Bin_align=275;
    // const static int nDimension=512;
    static int nDimension;
    static TString name_file_spe;
    static TString name_file_pde;
    static TString option_str;
    // const static int nDimension=4096;
    const static int waves_amplitude=512;
    const static int threshold=30;
    const static bool UsePDEDataToGatExtraSPE=true;

    //divide.C Setting
    const static bool useThreshold50 = true;

    //getCutRange.C Setting
    const static int sigma_selectSinglePhoton = 15;//注意这里因为没办法初始化float的const static，所以将0.5换成5，从而传入其中，一定要注意这是乘以10后的数

    //filter Settings
    const static bool  retain_FilterPeakOnBrae_removeWithPol2 =false;
    const static bool  retain_FilterPeak =false;
    const static bool  chopThePeak =true; //打开这个开关前必须先把retain_FilterPeakOnBrae调成false

    //choice_filter=1表示第一代滤波器（最开始用的滤波器），上面三种开关都有用
    //choice_filter=2表示第二代滤波器（将波形频谱去峰后得到的滤波器），上面三种开关失效
    //choice_filter==3表示第三代滤波器（将原始波形频谱直接得到的滤波器，再去除峰）
    const static int  choice_filter=3;
    
    //pdf and root name_option
    TString name_RawWaveSelectOption;
    TString name_AverageOption;
    TString name_WhichFilter;
    TString name_FilterOption;
    static TString name_RootFilePath;
    static TString name_WorkDir;
    static TString name_PdfDir;
    static TString name_RawDataDir;



};


void Initialiaze_pars_waves(TString name_filePath_SPE )
{
    pars_waves parsWaves;
    /////////Get the nDimension of waveform and the filename into struct pars_waves////////////
    parsWaves.name_file_spe = name_filePath_SPE(name_filePath_SPE.Last('/') + 1, 30);
    parsWaves.name_RawDataDir= name_filePath_SPE(0, name_filePath_SPE.Last('/'));
    parsWaves.name_RawDataDir= parsWaves.name_RawDataDir(0,parsWaves.name_RawDataDir.Last('/')+1);

    TString name_file_rid_last_num = parsWaves.name_file_spe(0, parsWaves.name_file_spe.Last('_'));
    TString str_nDimension = name_file_rid_last_num( name_file_rid_last_num.Last('_')+1,10);
    parsWaves.nDimension = str_nDimension.Atoi();
    cout<< "nDimension:      "<<parsWaves.nDimension<<endl;
    cout << "File name:       " << parsWaves.name_file_spe << endl;
    ////////////////////////////////////////////////////////////////////////////////////////////



    /////////Set useful dirtionary to pars_waves/////////////////////////////////
    parsWaves.name_RootFilePath="./RootFile/";
    parsWaves.name_PdfDir="./output_pdf/";
    system("mkdir "+parsWaves.name_RootFilePath);
    system("mkdir "+parsWaves.name_PdfDir);
    ////////////////////////////////////////////////////////////////////////////

}
