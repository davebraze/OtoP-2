Author: Tao Gong (Haskins Laboratories, New Haven CT, USA)

This is a demo of using MikeNet to simulate an Orthography to Phonology neural network model.

To compile it, type: make -f Makefile. It will generate the exe file called benchmark
To clean it, type: make -f Makefile clean. It will clear the exe file

To run it, put the exe file, para.txt parameter file, phon.txt, and trainexp_full.txt into the same directory
    para.txt has the following format:
	// Network Parameters
	1	// int _tai;
	7	// int _tick;
	0.25	// double _intconst;
	1e-3	// double _epsi;
    The first line is a comment line; in each of the following lines, there is the value of paramter + \t + // type and name of the parameter.
    One can easily change the value of each parameter to fit in new condition.

There are sevearl ways of running the model:
1) using exe file, type: ./benchmark. It will ask user to input an integer folder name, and the result files will be stored there. 
    Results include: 
	seed.txt: store random seed in that run;
    	weights.txt.gz: zipped connection weights of the trained network;
    	output.txt: network parameters and training and testing errors at each sampling point of the training;
    	itemacu_tr.txt: item-based accuracy based on the training data (training_examples.txt) at each sampling point;   
    	itemacu_te.txt: item-based accuracy based on the testing data (so far same as the training data) at each sampling point;
    One can specify some running parameters, like: .\benchmark -seed SEED -iter ITER -rep REP -met MET -vthres VTHRES
   	SEED: random seed to be used in that run;
   	ITER: number of total iterations;
   	REP: iterations for recording the results;
   	MET: method for determining which phonemen matches the activation;
   	VTHRES: if MET is 1, the bit difference threshold for determining which phoneme matches the activation;
    There are default values for these running parameters, and one can specify all or only some of these parameters using the key words "-*" 

2) Using shell command, put SerRunLoc.sh into the same folder with the exe file, para.txt, phon.txt, and trainexp_full.txt
    On-screen outputs will be stored in *.log files.
    type: sh SerRunLoc.sh NUM ITER REP MET VTHRES
   	NUM: number of runs to be conducted, each using a random seed; 
   	The other parameters are the same as above and optional.

3) Using Yale HPC: 
    On-screen outputs will be stored in *.log files.
    a) Using Grace: MikeNet has to be installed there first;
    	1. copy *.c, *.h, Makefile, para.txt, phon.txt, trainexp_full.txt, msf.sh into the working directory;
    	2. load a module for GCC: $ module load Langs/GCC
    	3. load a module for MikeNet: $ module load Libs/MikeNet
    	4. compile the source code to exe file: $ make -f Makefile (to clean: make -f Makefile clean)  
    	Note that: once the code is compiled, one can use the same exe file with different para.txt in different running conditions, without compiling the code again!	
    	5. set up the parallel running via msf.sh:
    	6. use "chmod +rwx msf.sh" to change msf.sh permission
    	7. set up tasklist.txt:
    		cd ~/workDirec; ./msf.sh 1 5000 1000
    		cd ~/workDirec; ./msf.sh 2 5000 1000
    		cd ~/workDirec; ./msf.sh 3 5000 1000
    		cd ~/workDirec; ./msf.sh 4 5000 1000
    		...
    	This example runs 4 simulations, each having 5000 iterations and sampling results every 1000 iterations.
    
    	You can use: sh genTasklist.sh NUMRUN WORKDIREC ITER REP MET VTHRES to automatically generate tasklist.txt
        		NUMRUN: total number of runs;
        		WORKDIREC: working directory of the code;
        		The other parameters are the same as above and optional.
   
    	8. run the results via SimpleQueue
   		$ module load Tools/SimpleQueue
    		$ sqCreateScript -n 4 -W 24:00 tasklist.txt > job.sh
    		$ bsub < job.sh
    	It recruits 4 nodes (8 cpus each) to run the work for 24 hours. Note that the total number of runs has to be a multipler of 32.
   	9. check status: $ bjobs; To kill a job: $ bkill job_ID

    b) Using Omega: MikeNet has to be installed there first;
    	1. same as above.
    	2. load a module for GCC: $ module load Langs/GCC/4.5.3
    	3-6. same as above.
    	7. set up tasklist.txt: add "module load Langs/GCC/4.5.3;" in front of each line in tasklist.txt
    
    	You can use: sh genTasklist_Omega.sh NUMRUN WORKDIREC ITER REP MET VTHRES to automatically generate tasklist.txt
        		NUMRUN: total number of runs;
        		WORKDIREC: working directory of the code;
        		The other parameters are the same as above and optional

    	8. run the results via SimpleQueue
    		$ module load Tools/SimpleQueue
    		$ sqCreateScript -n 3 -w 24:00:00 tasklist.txt > job.sh
    		$ qsub < job.sh
    	It recruits 3 nodes (8 cpus each) to run the work for 24 hours. Note that the total number of runs has to be a multipler of 24.
    	9. check status: $ qstat -u USER; to kill a job: $ qdel job_ID