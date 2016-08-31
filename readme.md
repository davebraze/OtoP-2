---
Author: Tao Gong
Title: Document Use of OtoP Simulation
Affiliation: Haskins Laboratories
output: pdf_document
---

This repository contains code for a neural network (implemented in
[Mikenet](http://www.cnbc.cmu.edu/~mharm/research/tools/mikenet/))
that learns Orthography to Phonology mappings.

The rest sketches how to compile and run the simulation in three contexts: on a local workstation; on the Yale HPC's Grace cluster; on the Yale HPC's Omega cluster.

## Local Workstation

#### Compilation

The model is compiled and run under Linux (Mint 17), with Mikenet package is installed. The source codes include: model.h, model.c, and OtoP.c.

To compile an executable file, type: `make -f Makefile` in command line. This will generate the exe file
called *OtoP*.

To clean previous exe file, type: `make -f Makefile clean`. This step is optional.

#### Running the Simulation

In order to run a simulation, put the exe file (OtoP), parameter file
(para.txt), phoneme dictionary (phon.txt), and training examples
(trainexp_full.txt) into the same directory.

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

There are two ways of running the model:

1. Using exe file, type: ./OtoP. It will ask user to input an integer
    folder name, and the simulation results are stored there. While running the model, there are on-screen outputs showing the error and training/testing accuracies at each sampling points.  Results include:

	* seed.txt: store random seed in that run;

    * weights.txt.gz: zipped connection weights of the trained
      network;

    * output.txt: network parameters and training and testing errors
      at each sampling point of the training;

    * itemacu\_tr.txt: item-based accuracy based on the training data
      (training\_examples.txt) at each sampling point;

    * itemacu\_te.txt: item-based accuracy based on the testing data
      (so far same as the training data) at each sampling point;


   One can specify some running parameters as command line arguments
   when calling the executable (OtoP):
   `./OtoP -seed SEED -iter ITER -rep REP -samp SAMP -met MET -vthres VTHRES`

   There are default values for each parameter, so one can
   specify all or only some of them.

	  * SEED: random seed to be used in that run. Default value is 0

 	  * ITER: number of total iterations. Default value is 50000

 	  * REP: iterations for recording the results; default value is 1000

 	  * SAMP: sampling method (0: liner; 1: logarithm-like). If SAMP
         is set to 1, REP is no longer useful. Default value is 0

 	  * MET: method for determining which phonemen matches the
         activation: 0: based on the smallest vector-based Euclidean distance between the output and the phoneme; 1, based on whether the bit differences between the output and the phonemen are all within certain threshold (VTHRES). Default value is 0

 	  * VTHRES: if MET is 1, the bit difference threshold for
         determining which phoneme matches the activation. Default value is 0.5

2. Using shell script, put SerRunLoc.sh into the same folder with the
   exe file, para.txt, phon.txt, and trainexp_full.txt. On-screen
   outputs will be stored in *.log files.

   This way of running allows user to set up a number of runs each having a different random seed. The computer will start each run serially, and store the results in the corresponding subfolders (1 to N, N is the number of runs preset).

   type: `sh SerRunLoc.sh NUM ITER REP SAMP MET VTHRES`

	* NUM: number of runs to be conducted, each using a different random seed;

	  * The other parameters are the same as above and optional.

## Yale HPC

    On-screen outputs as in the first way of running will be stored in *.log files.

### Using Grace

#### Compilation

	Note that MikeNet has to be installed on Grace before it can be
	linked into an executable.

      1. copy *.c, *.h, Makefile, para.txt, phon.txt,
         trainexp_full.txt, msf.sh into the working directory;

      2. load a module for GCC: $ module load Langs/GCC

      3. load a module for MikeNet: $ module load Libs/MikeNet

      4. compile the source code to exe file: $ make -f Makefile (to
         clean: make -f Makefile clean). Once the code is compiled,
         use the same exe file with different para.txt in different
         conditions, no need recompilation.

#### Running the Simulation

   	  1. set up the parallel running via msf.sh:

   	  2. use "chmod +rwx msf.sh" to change msf.sh permission

   	  3. set up tasklist.txt with commands like this (repeat as needed):

	     ```
   	     cd ~/workDirec; ./msf.sh 1 5000 1000
   		 cd ~/workDirec; ./msf.sh 2 5000 1000
   		 cd ~/workDirec; ./msf.sh 3 5000 1000
   		 cd ~/workDirec; ./msf.sh 4 5000 1000
   		 ```

   		And so on.

   		As shown, this example would run 4 simulations, each having
        5000 iterations and sampling results every 1000 iterations.

      	You can use the a command like the following to automatically
          generate tasklist.txt:

		`sh genTasklist.sh NUMRUN WORKDIREC ITER REP SAMP MET VTHRES`

		* NUMRUN: total number of runs;

		* WORKDIREC: working directory of the code;

		* The other parameters are the same as above and
          optional. TAO, CLARIFY THE ANTECEDENT FOR 'above'.

      4. run the results via SimpleQueue

	     ```
   	     module load Tools/SimpleQueue
    	 sqCreateScript -n 4 -W 24:00 tasklist.txt > job.sh
    	 bsub < job.sh
		 ```

    	 As shown, this example recruits 4 nodes (8 cpus each) to run
         for 24 hours. Note that the total number of runs has to be a
         multipler of 32.

   	  5. You can check job status thus: `bjobs; To kill a job: $ bkill job_ID`

### Using Omega

#### Compilation

	Note that MikeNet has to be installed on Omega before it can be
	linked into an executable.


       1. same as for Grace.

	   2. load the module for GCC: `module load Langs/GCC/4.5.3`

	   3. same as for Grace.
	   4. same as for Grace.

#### Running a Simulation

	   1. same as for Grace.
	   2. same as for Grace.

       3. set up tasklist.txt: add "module load Langs/GCC/4.5.3;" in
          front of each line in tasklist.txt

        You can use a command like the following to automatically generate
        tasklist.txt:

		```
		sh genTasklist_Omega.sh NUMRUN WORKDIREC ITER REP SAMP MET VTHRES
		```

        * NUMRUN: total number of runs;
        * WORKDIREC: working directory of the code;

        The other parameters are the same as for Grace, and are optional.


      4. run the results via SimpleQueue
	     ```
         module load Tools/SimpleQueue
    	 sqCreateScript -n 3 -w 24:00:00 tasklist.txt > job.sh
    	 qsub < job.sh
         ```

    	 As shown, this example recruits 3 nodes (8 cpus each) to run
         for 24 hours. Note that the total number of runs has to be a
         multipler of 24.

      5. check status of a job this:

	  ```
	  qstat -u USER; to kill a job: $ qdel job_ID
	  ```
