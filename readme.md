---
Author: Tao Gong
Title: Document Use of OtoP Simulation
Affiliation: Haskins Laboratories
output: pdf_document
---

This repository contains a Mikenet implementation of a neural network
that learns Orthography to Phonology mappings.

## Compilation

To compile it, type: make -f Makefile. It will generate the exe file
called `benchmark`.

To clean it, type: `make -f Makefile clean`. It will clear the exe file.

## Run

To run it, put the exe file (OtoP), parameter file (para.txt), phoneme
dictionary (phon.txt), and training examples (trainexp_full.txt) into
the same directory.

para.txt has the following format:

> // Network Parameters

> 1	// int _tai;

> 7	// int _tick;

> 0.25	// double _intconst;

> 1e-3	// double _epsi;

> ...

The first line is a comment line; in each of the following lines, the
format is: value + \t + // type and name of the parameter.  One can
easily change the value of each parameter to fit in new condition.

There are sevearl ways of running the model:

1. Using exe file, type: ./OtoP. It will ask user to input an integer folder name, and the results are stored there.
    Results include:

	* seed.txt: store random seed in that run;

    * weights.txt.gz: zipped connection weights of the trained
      network;

    * output.txt: network parameters and training and testing errors
      at each sampling point of the training;

    * itemacu\_tr.txt: item-based accuracy based on the training data
      (training\_examples.txt) at each sampling point;

    * itemacu\_te.txt: item-based accuracy based on the testing data
      (so far same as the training data) at each sampling point;


   One can specify some running parameters, like:
   `./OtoP -seed SEED -iter ITER -rep REP -samp SAMP -met MET -vthres VTHRES`

 	  * SEED: random seed to be used in that run;

 	  * ITER: number of total iterations;

 	  * REP: iterations for recording the results;

 	  * SAMP: sampling method (0: liner; 1: logarithm-like). If SAMP
         is set to 1, REP is no longer useful.

 	  * MET: method for determining which phonemen matches the
         activation;

 	  * VTHRES: if MET is 1, the bit difference threshold for
         determining which phoneme matches the activation;

   There are default values for these running parameters, and one can
   specify all or only some of them using the key words "-*"

2. Using shell command, put SerRunLoc.sh into the same folder with the
   exe file, para.txt, phon.txt, and trainexp_full.txt. On-screen
   outputs will be stored in *.log files.

   type: `sh SerRunLoc.sh NUM ITER REP SAMP MET VTHRES`

	* NUM: number of runs to be conducted, each using a random seed;

   The other parameters are the same as above and optional.

3. Using Yale HPC:

    On-screen outputs will be stored in *.log files.

	1. Using Grace: MikeNet has to be installed there first;

      1. copy *.c, *.h, Makefile, para.txt, phon.txt,
         trainexp_full.txt, msf.sh into the working directory;

      2. load a module for GCC: $ module load Langs/GCC

      3. load a module for MikeNet: $ module load Libs/MikeNet

      4. compile the source code to exe file: $ make -f Makefile (to
         clean: make -f Makefile clean). Once the code is compiled,
         use the same exe file with different para.txt in different
         conditions, no need recompilation.

   	  5. set up the parallel running via msf.sh:

   	  6. use "chmod +rwx msf.sh" to change msf.sh permission

   	  7. set up tasklist.txt with commands like this (repeat as needed):

	     ```
   	     cd ~/workDirec; ./msf.sh 1 5000 1000
   		 cd ~/workDirec; ./msf.sh 2 5000 1000
   		 cd ~/workDirec; ./msf.sh 3 5000 1000
   		 cd ~/workDirec; ./msf.sh 4 5000 1000
   		 ```

   		And so on.

   		As shown, this example would run 4 simulations, each having
        5000 iterations and sampling results every 1000 iterations.

      	You can use:

		`sh genTasklist.sh NUMRUN WORKDIREC ITER REP SAMP MET VTHRES to automatically generate tasklist.txt`

		* NUMRUN: total number of runs;

		* WORKDIREC: working directory of the code;

		The other parameters are the same as above and optional.

      8. run the results via SimpleQueue

	     ```
   	     module load Tools/SimpleQueue
    	 sqCreateScript -n 4 -W 24:00 tasklist.txt > job.sh
    	 bsub < job.sh
		 ```

    	 As shown, this example recruits 4 nodes (8 cpus each) to run
         for 24 hours. Note that the total number of runs has to be a
         multipler of 32.

   	  9. You can check job status thus: `bjobs; To kill a job: $ bkill job_ID`


    2. Using Omega: MikeNet has to be installed there first;

       1. same as above.

	   2. load a module for GCC: `module load Langs/GCC/4.5.3`

	   3. same as above.
	   4. same as above.
	   5. same as above.
	   6. same as above.

       7. set up tasklist.txt: add "module load Langs/GCC/4.5.3;" in
          front of each line in tasklist.txt

        You can use a command like this to automatically generate
        tasklist.txt:

		```
		sh genTasklist_Omega.sh NUMRUN WORKDIREC ITER REP SAMP MET VTHRES
		```

        * NUMRUN: total number of runs;
        * WORKDIREC: working directory of the code;

        The other parameters are the same as above and optional


      8. run the results via SimpleQueue
	     ```
         module load Tools/SimpleQueue
    	 sqCreateScript -n 3 -w 24:00:00 tasklist.txt > job.sh
    	 qsub < job.sh
         ```

    	 As shown, this example recruits 3 nodes (8 cpus each) to run
         for 24 hours. Note that the total number of runs has to be a
         multipler of 24.

      9. check status of a job this:
	  ```
	  qstat -u USER; to kill a job: $ qdel job_ID
	  ```
