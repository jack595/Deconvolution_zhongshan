import ROOT
import root_numpy
import numpy as np
from scipy import signal
import matplotlib.pyplot as plt
import argparse
from scipy import interpolate

plt.style.use("/afs/ihep.ac.cn/users/l/luoxj/.local/Style/Paper.mplstyle")

parser = argparse.ArgumentParser(description='Deconvolution GetFilter')
parser.add_argument("--input",'-i',type=str, help="The name of file saving profiles in FULL PATH")
args = parser.parse_args()

name_file_FULLPATH:str = args.input
index_last_slash = name_file_FULLPATH.rfind('/')
path_RootFile = name_file_FULLPATH[:index_last_slash+1]
print(f"path_RootFile : {path_RootFile}")
name_file = name_file_FULLPATH[index_last_slash+1:]
index_cut_name_file= name_file.find("_Profile")
name_raw_file = name_file[:index_cut_name_file]
print(f"Raw File Name : {name_raw_file}")

useSpline = False


# f_profile = ROOT.TFile("/afs/ihep.ac.cn/users/l/luoxj/Deconvolution_zhongshan/RootFile/0_0_09-26-1021_1008_9034_Profile.root", "read")
f_profile = ROOT.TFile(name_file_FULLPATH, "read")
raw2D = f_profile.Get("h2D_raw_h_profile")
Noise2D = f_profile.Get("h2D_Modulus_noise")
# Noise2D.Draw()
v_raw = root_numpy.hist2array(raw2D)
v_raw= v_raw[1:]
v_noise = root_numpy.hist2array(Noise2D)
v_noise = v_noise[1:]
poly_order = 7
window_length = 151
# v_raw_smoothed = signal.savgol_filter(v_raw, window_length, poly_order)
# v_noise_smoothed = signal.savgol_filter(v_noise, window_length, poly_order)
# plt.figure("Signal+Noise")
# plt.plot(v_raw_smoothed)
# plt.plot(v_raw)
# plt.figure("Noise")
# plt.plot(v_noise)
# plt.plot(v_noise_smoothed)
# # print(v_raw)
# plt.figure("Signal_Smooth")
# v_signal = v_raw_smoothed - v_noise_smoothed
# plt.plot(v_signal)
# plt.plot(v_raw_smoothed)
# plt.plot(v_noise_smoothed)
plt.figure("Signal", figsize=(12,9))
line_raw = plt.plot(v_raw, label="Signal + Noise")
line_noise = plt.plot(v_noise, label="Noise")
v_signal_NoSmooth = v_raw - v_noise
line_signal_before_smooth = plt.plot(v_signal_NoSmooth, label="Signal Before Smoothing")
v_signal_Smooth = signal.savgol_filter(v_signal_NoSmooth, window_length, poly_order)
# plt.plot(v_signal_Smooth)

index = np.where(v_signal_Smooth<0)[0][0]
v_signal_Smooth[index:]=0
# v_signal_Smooth[50:]=0
line_signal_smoothed = plt.plot(v_signal_Smooth, label="Smoothed Signal")

# plt.legend(handles=[line_raw, line_noise, line_signal_before_smooth, line_signal_smoothed])

plt.title("Signal and Noise Profile in Frequency Region")
plt.xlabel("Frequency [ MHz ]")
plt.ylabel("Complex Modulus")
plt.legend( loc='upper right')

plt.savefig("./output_pdf/DepriveNoiseToGetFilter/"+name_raw_file+"_profile.png")

# filter_Wiener = v_signal_Smooth**2/v_raw**2
filter_Wiener = v_signal_Smooth**2/(v_noise**2+v_signal_Smooth**2)
figure_filter = plt.figure("Filter", figsize=(12,9))

# filter_Wiener[:20] = 1
plt.plot(filter_Wiener)

#########Smooth filter############


if useSpline:
    len_filter = len(filter_Wiener)
    list_to_delete = []
    x_filter = np.arange(len(filter_Wiener))
    for i in range(len(filter_Wiener)):
        if i <len(filter_Wiener)-1 and i>30:
            if filter_Wiener[i+1]>filter_Wiener[i]+0.05:
                start_save = i+1+np.where(filter_Wiener[i+1:]<filter_Wiener[i])[0][0]
                list_to_delete+=range(i+1, start_save )
                if i<start_save:
                    continue
    print(list_to_delete)
    filter_Wiener = np.delete( filter_Wiener, list_to_delete)
    x_filter = np.delete( x_filter, list_to_delete)

    # plt.plot(filter_Wiener)
    tck = interpolate.splrep(x_filter,filter_Wiener, s=0)
    filter_Wiener = interpolate.splev( np.arange(len_filter) ,tck, der=0)
    filter_Wiener = signal.savgol_filter(filter_Wiener, 33 , 2 )
else:
    # filter_Wiener[60:]=0
    filter_Wiener = signal.savgol_filter(filter_Wiener, 33 , 2 )
    filter_Wiener[filter_Wiener>1] = 1
    index_final_1 = np.where(filter_Wiener==1)[0]
    if len(index_final_1)>0:
        filter_Wiener[:index_final_1[-1]] = 1





filter_zero_index = np.where((filter_Wiener<0))[0]
if len(filter_zero_index)>0:
    filter_Wiener[filter_zero_index[0]:]=0
plt.plot(filter_Wiener)
plt.title("Filter")
plt.xlabel("Frequency [ MHz ]")
plt.ylabel("Component")

plt.savefig("./output_pdf/DepriveNoiseToGetFilter/"+name_raw_file+"_filter.png")

h_filter = ROOT.TH1D("filter", "filter", filter_Wiener.shape[0], 0, filter_Wiener.shape[0] )
_ = root_numpy.array2hist(filter_Wiener, h_filter)
print("Saving Filter to "+path_RootFile+name_raw_file+"_filter.root")
f_filter = ROOT.TFile(path_RootFile+name_raw_file+"_filter.root","recreate")
h_filter.Write()

plt.show()
# input("enter")
f_filter.Close()
f_profile.Close()
