Author: Tao Gong (Haskins Laboratories, New Haven CT, USA)

This is a simple demo of using MikeNet to simulate an Orthography to Phonology neural network model.

To compile it, use: make -f Makefile. It will generate the exe file called benchmark
To clean it, use: make -f Makefile clean. It will clear the exe file

To run it using exe file, use: ./benchmark. It will ask user to input an integer folder name, and the result files will be stored there. 
Result files include: 
    seed.txt: store random seed in that run;
    weights.txt.gz: zipped connection weights of the trained network;
    output.txt: network parameters and training and testing errors at each sampling point of the training;
    itemacu_tr.txt: item-based accuracy based on the training data (training_examples.txt) at each sampling point;   
    itemacu_te.txt: item-based accuracy based on the testing data (so far same as the training data) at each sampling point;

One can also specify some parameters, like: benchmark -seed SEED -iter ITER -rep REP -met MET -vthres VTHRES
   SEED: random seed to be used in that run;
   ITER: number of total iterations;
   REP: iterations for recording the results;
   MET: method for determining which phonemen matches the activation
   VTHRES: if MET is 1, the bit difference threshold for determining which phoneme matches the activation
On-screen outputs will be stored in *.log files. To change parameters of the network, see the macros in model.h

To run it using shell command, use: sh SerRunLoc.sh NUM ITER REP
   NUM: number of runs to be conducted, each using a random seed;
   ITER and REP: same as above; ITER and REP are optional.

To run it using Yale HPC (Grace): MikeNet has to be installed there first;
1. copy *.c, *.h, Makefile, phonemes.txt, training_examples.txt into the working directory;
2. load a module for GCC: $ module load Langs/GCC
3. load a module for MikeNet: $ module load Libs/MikeNet
4. compile the source code to exe file: $ make -f Makefile (to clean: make -f Makefile clean)
5. set up the parallel running via msf.sh:
    #!/bin/bash
    rm -fr $1
    if [ -z $2 ]; then
        echo $1 | time ./benchmark > $1.log
    else
        echo $1 | time ./benchmark -iter $2 -rep $3 > $1.log
    fi
6. use "chmod +rwx msf.sh" to change msf.sh permission
7. set up tasklist.txt:
    cd ~/workDirec; ./msf.sh 1 5000 1000
    cd ~/workDirec; ./msf.sh 2 5000 1000
    cd ~/workDirec; ./msf.sh 3 5000 1000
    cd ~/workDirec; ./msf.sh 4 5000 1000
    ....
    This example runs 4 simulations, each having 5000 iterations and sampling results every 1000 iterations.
8. run the results via SimpleQueue
    $ module load Tools/SimpleQueue
    $ sqCreateScript -n 4 -W 24:00 tasklist.txt > job.sh
    $ bsub < job.sh
    It recruits 4 nodes (8 cpus each) to run the work for 24 hours. Note that the total number of runs has to be a multipler of 32.
9. check status: $ bjobs; To kill a job: $ bkill job_ID

To run it using Yale HPC (Omega): MikeNet has to be installed there first;
1. same as above.
2. load a module for GCC: $ module load Langs/GCC/4.5.3
3. same as above.
4. same as above.
5. same as above.
6. same as above.
7. set up tasklist.txt: add "module load Langs/GCC/4.5.3;" in front of each line in tasklist.txt
8. run the results via SimpleQueue
    $ module load Tools/SimpleQueue
    $ sqCreateScript -n 3 -w 24:00:00 tasklist.txt > job.sh
    $ qsub < job.sh
    It recruits 3 nodes (8 cpus each) to run the work for 24 hours. Note that the total number of runs has to be a multipler of 24.
9. check status: $ qstat -u USER; to kill a job: $ qdel job_ID