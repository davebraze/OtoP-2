---
Author: Tao Gong
Title: Document Use of OtoP Simulation
Affiliation: Haskins Laboratories
output: pdf_document
---

This repository contains code for a neural network (implemented in
[Mikenet](http://www.cnbc.cmu.edu/~mharm/research/tools/mikenet/))
that learns Orthography to Phonology mappings.

The rest of this email sketches how to compile and run the simulation
in three contexts: on a local workstation; on the Yale HPC's Grace
cluster; on the Yale HPC's Omega cluster.

## Local Workstation

#### Compilation

TAO, PLEASE SUMMARIZE THE OS AND TOOL CHAIN THAT IS ASSUMED HERE:
LINUX DISTRIBUTION, COMPILER, AND SO FORTH.

To compile an executable file, type: `make -f Makefile`. This will generate the exe file
called *benchmark*.
TAO, SEVERAL PLACES BELOW YOU SEEM TO ASSUME THAT
THE EXE FILE IS CALLED OtoP. PLEASE RECONCILE.

TAO, PLEASE SAY SOMETHING ABOUT WHY *make clean* MIGHT BE NECESSARY.
To clean it, type: `make -f Makefile clean`. It will clear the exe file.

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

There are several ways of running the model:
TAO, BY 'SEVERAL' DO YOU REFER TO THE TWO WAYS DETAILED BELOW? IF SO
THEN SAY 'two ways'

1. Using exe file, type: ./OtoP. It will ask user to input an integer
    folder name, and the results are stored there.  Results include:

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

	TAO, PLEASE INCLUDE THE DEFAULT VALUES FOR EACH PARAMETER IN THE
    LIST BELOW.

 	  * SEED: random seed to be used in that run;

 	  * ITER: number of total iterations;

 	  * REP: iterations for recording the results;

 	  * SAMP: sampling method (0: liner; 1: logarithm-like). If SAMP
         is set to 1, REP is no longer useful.

 	  * MET: method for determining which phonemen matches the
         activation;

 	  * VTHRES: if MET is 1, the bit difference threshold for
         determining which phoneme matches the activation;

2. Using shell script, put SerRunLoc.sh into the same folder with the
   exe file, para.txt, phon.txt, and trainexp_full.txt. On-screen
   outputs will be stored in *.log files.

   TAO, PLEASE SAY WHY ONE MIGHT PREFER TO USE A SHELL SCRIPT
   (SerRunLoc.sh) TO START A SIMULATION, RATHER THAN RUNNING THE
   EXECUTABLE DIRECTLY.

   type: `sh SerRunLoc.sh NUM ITER REP SAMP MET VTHRES`

	* NUM: number of runs to be conducted, each using a random seed;

	  TAO, DO YOU MEAN TO SAY 'each using a *different* random seed'?
	  PLEASE CLARIFY.

    * The other parameters are the same as above and optional.

## Yale HPC

    On-screen outputs will be stored in *.log files.
	TAO, WHAT IS MEANT BY 'ON-SCREEN' HERE?

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
