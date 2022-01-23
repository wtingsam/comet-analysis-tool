CDC Calibration package.

<a href="https://gitlab.in2p3.fr/samwong/CDC_Calibration" target="_blank">https://gitlab.in2p3.fr/samwong/CDC_Calibration</a>

# **Brief facts**

------------------------
- Application 
  - Inside app/, there are several offical applications.
    - get1DXTrelation.cxx  getPreRoot.cxx main.cxx simTrack.cxx (Name will be changed later)
  - User can also develop their own application. 

- Input
  - Input Wire Configuration Root File.
    - It should exist, when you clone from the git.
      The mapping is created by Moritsu Manabu. You can find it at ELOG #560
  - Input Run Log Root File 
    - This part is still under construction, please do not care about it for a moment
      It should exist, when you clone from the git.
  - Input Root File 
    - Structure of the root file is defined by the CRT stage 1-2.
- Output
  - Output pre-analysis Root File 
    - The purpose of this root file is for further analysis.
  - Output track Root File 
    - The purpose of this root file is to get the result of track reconstruction from pre-analysis root file.

# **How to use**

------------------------

## **Before start**

```
source setup.sh 
```

Then you have to choose the localtion of ``input`` and ``output`` files

Produce the beta z mapping text file using macro
```
root 
[1] .L genBetaZTable.c+
[2] generate("../info/chanmap_20160814.root",100,"betaZ_100bin.txt")
```
Then, you will obtain a table of beta and Z with 100 bin. 
This data is around 60 MB, please do not upload it to git server. 

## **pre-track*


For Osaka university user, 
```
scp username@192.168.0.83:/home/hiroki/CRT/root1/run_000061.root $(YourPath)/CDC_Calibration/root-input/ #Sakura server
```
For users who cannot find the raw root files,  please contact the admins.

After you have source the environment, under ${CCWORKING_DIR}, do
```
make
./bin/pre-track -h

[options]
	 -e
		 choose experiment:[crtsetup2(default)]
         -c
		 choose configure file for experiments: [experiemnt-config.txt(default)]
	 -n
		 set maximum event number [0 as default (no limit)]
	 -r
		 force a run number
	 -i
		 set an input file
	 -p
		 set event level information print modulo [1000 as default]
	 -h
		 help message 

```
The default setup is CRT setup 1-2.
Usually, only `` -r `` and  ``-i`` option are used.

## **Track reconstruction**

You just need to do
```
./bin/track -h

[options]
	 -r
		 force a run number
	 -I
		 set a iteration step
	 -f
		 set a file id
	 -t
		 set test layer 
	 -n
		 set maximum number of files [0 as default (no limit)]
	 -i
		 set an input file
	 -p
		 set event level information print modulo [1000 as default]
	 -d
		 set draw event option [0:default, 1:draw]
	 -T
		 set calibration t0 flag [0:default, 1:Do it
	 -h
		 help message 
```

1. Testlayer: To exclude layer for resolution study
2. iFile and numOfFiles
   - iFile: FileID 
   - numOfFiles: Number of files you want to slice 
3. Debug: (Input 0 will read all the events)
   i. Enter a event number 
   ii. Output a event display for this event

e.g.
```
./track -r 61 -I 0 -f 0 -t 10 -n 50 -d 1
```
Then we can do parallel job submission analysis at other server. 
There are other applications in the ``app/``.
It includes the resolution analysis, x-t analysis etc. 

## **Job Submission**

```
Please change the path inside the script to <your path>
./jobs_tracking.sh <runNo><iteration><numOfFiles><startLayer><endLayer> 
numOfFiles = numOfJobs
```

## ** Example of calibration procedure ** 

This is the simpliest way of running the calibration procedure, notice that the procedure has to be followed. 

1. bin/pre-track -e crtsetup2 -i input/run_000061.root -r 61 -n 10000 
2. bin/track -r 61 -I 0 -t 10 -f 0 -n 20 -e crtsetup2 -d 0 
3. bin/getAvgXt -r 61 -I 0 -f 10 -t 11 // From layer 10 to layer 11
4. bin/getResVsR -r 61 -I 4 -t 9

## **For developer**

- Please submit a merge Request