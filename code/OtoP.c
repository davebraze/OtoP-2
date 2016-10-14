#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mikenet/simulator.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "model.h"

#define _FileLen 100
#define _Rand0_1 rand()/(RAND_MAX+1.0)

// parameters for recording timepoints, total iteration, and seed;
unsigned int _seed=0;
unsigned int _iter=5e4;	// total number of training;
unsigned int _rep=1e3;	// sampling frequency; 
int _ptop=0;	// whether involve ptop training (1) or not (0);
unsigned int _iter_ptop=5e4;	// total number of ptop training;
unsigned int _rep_ptop=1e3; // sampling frequency during ptop training;
int _samp_method=0;	// method to do sampling; linear (0) or logarithm-like (1) sampling;
int _v_method=0;	// method to translate output; based on smallest Euclidean-distance (0) or vector range (1);
double _v_thres=0.5;	// if _v_method=0, the threshold used for vector range method;
int _recVec=0;	// whether recording output and hidden layers' vectors (1) or not (0);

// functions to get parameter values;
void getpara_unsignedint(FILE *f, unsigned int *para)
{ // get unsigned integer paramter;
	assert(f!=NULL);
	char *line=malloc(_LineLen*sizeof(char)); assert(line!=NULL);
	char sep[]="\t", *token=NULL;

	fgets(line, _LineLen, f); token=strtok(line, sep);
   	while(token!=NULL) { *para=atoi(token); token=NULL; }
	free(line); line=NULL;
	token=NULL;
}

void getpara_int(FILE *f, int *para)
{ // get integer paramter;
	assert(f!=NULL);
	char *line=malloc(_LineLen*sizeof(char)); assert(line!=NULL);
	char sep[]="\t", *token=NULL;

	fgets(line, _LineLen, f); token=strtok(line, sep);
   	while(token!=NULL) { *para=atoi(token); token=NULL; }
	free(line); line=NULL;
	token=NULL;
}

void getpara_double(FILE *f, double *para)
{ // get double paramter;
	assert(f!=NULL);
	char *line=malloc(_LineLen*sizeof(char)); assert(line!=NULL);
	char sep[]="\t", *token=NULL;

	fgets(line, _LineLen, f); token=strtok(line, sep);
   	while(token!=NULL) { *para=atof(token); token=NULL; }
	free(line); line=NULL;
	token=NULL;
}

void getpara_string(FILE *f, char **s)
{	assert(f!=NULL);
	char *line=malloc(_LineLen*sizeof(char)); assert(line!=NULL);
	char sep[]="\t", *token=NULL;

	fgets(line, _LineLen, f); token=strtok(line, sep);
   	while(token!=NULL) 
		{ *s=malloc((strlen(token)+1)*sizeof(char)); assert(*s!=NULL); 
		  strcpy(*s, token); token=NULL; 
		}
	free(line); line=NULL;
	token=NULL;
}

void readpara(void)
{ // read parameters from para.txt;
	FILE *f=NULL; 
	char *line=malloc(_LineLen*sizeof(char)); assert(line!=NULL);

	if((f=fopen("para.txt","r+"))==NULL) { printf("Can't open OverAllPara.txt\n"); exit(1); }

	fgets(line, _LineLen, f);	  // read: // Network Parameters
	getpara_int(f, &_tick_OtoP);	// read _tick_OtoP; number of ticks in one epoch (trial) of otop training; different types of training can happen in different ticks;
	getpara_int(f, &_tick_PtoP);	// read _tick_PtoP; number of ticks in one epoch (trial) of ptop training; different types of training can happen in different ticks;
	getpara_double(f, &_intconst);	// read _intconst; default value is 0.25;
	getpara_int(f, &_tai);	// read _tai; default value is 1.
	getpara_double(f, &_epsi);	// read _epsi; episilon value for the activation curve; default value is 0.001;
	getpara_int(f, &_acttype);	// read _acttype; // LOGISTIC_ACTIVATION (0), TANH_ACTIVATION (1), FAST_LOGISTIC_ACTIVATION (2), LINEAR_ACTIVATION (3), or STEP_ACTIVATION (4); 
	getpara_int(f, &_errortype); // int _errortype; SUM_SQUARED_ERROR (1) or CROSS_ENTROPY_ERROR (2)
	getpara_int(f, &_weightnoisetype);	// int _weightnoisetype; NO_NOISE (0), ADDITIVE_NOISE (1), or MULTIPLICATIVE_NOISE (2)
	getpara_double(f, &_weightnoise);	// double _weightnoise; noise on connection weights;
	getpara_double(f, &_actnoise);	// double _actnoise; activation noise;
	getpara_double(f, &_inputnoise);	// double _inputnoise; input noise;
	getpara_double(f, &_errrad);	// read _errrad; error radius, errors less than it are counted as zero; default value is 0.1;
	getpara_double(f, &_range);	// read _range; range of initial weights, the initially randomized weights are positive and negative _range;
	getpara_int(f, &_OrthoS);	// read _OrthoS; size (number of nodes) of the orthographical layer;
	getpara_int(f, &_HidS);	// read _HidS; size of the hidden layer between the orthographical and phonological layers;
	getpara_int(f, &_PhonoS);	// read _PhonoS; size of the phonological layer; For Harm & Seidenberg 1999, 77 (7*11); For Harm 1998, 175 (7*25);
	getpara_int(f, &_PhoHidS);	// read _PhoHidS; size of the hidden layers between phonological layers, this is the cleanup layer;

	fgets(line, _LineLen, f);	// read: // Parameters for phonemes
	getpara_int(f, &_pho_features);	// read _pho_features; number of features in each phoneme; For Harm & Seidenberg 1999, 11; For Harm 1998, 25;
	getpara_int(f, &_pho_number);	// read _pho_number; number of phonemes in the dictionary; For Harm & Seidenberg 1999, 36 (24+11+1); For Harm 1998, 39 (24+10+4+1);

	fgets(line, _LineLen, f);	// read: // Parameters for file names storing phonological dictionary and training and testing examples
	getpara_string(f, &_phoF);	// read _phoF; file name of the phoneme dictionary, which is a list of phonemes and their feature values;
	getpara_string(f, &_exTrF);	// read _exTrF; file name of the training examples; TrEm_HarmSeidenberg1999.txt or TrEm_Harm1998.txt;
	getpara_string(f, &_exTeF);	// read _exTeF; file name of the testing examples; TeEm_HarmSeidenberg1999.txt or TeEm_Harm1998.txt or TE.txt (for nonwords); 
	getpara_string(f, &_exTrF_PtoP);	// read _exTrF_PtoP; file name of the training examples training phonological cleanup units; TrEM_PtoP_HarmSeidenberg1999.txt or TrEM_PtoP_Harm1998.txt; 

	fgets(line, _LineLen, f);	// read: // Parameters for running
	getpara_unsignedint(f, &_seed);	// read _seed; random seed for each run; if _seed=0, use random seed;
	getpara_unsignedint(f, &_iter);	// read _iter; total number of training;
	getpara_unsignedint(f, &_rep);	// read _rep; sampling frequency during training;
	getpara_int(f, &_ptop);	// read _ptop; whether involve ptop training (1) or not (0);
	getpara_unsignedint(f, &_iter_ptop);	// read _iter_ptop; total number of ptop training;
	getpara_unsignedint(f, &_rep_ptop);	// read _rep_ptop; sampling frequency during ptop training;
	getpara_int(f, &_samp_method);	// read _samp_method; sampling method; linear (0) or logarithm-like (1) sampling;
	getpara_int(f, &_v_method);	// read _v_method; method to translate ouput during training; based on smallest Euclidean-distance (0) or vector range (1);
	getpara_double(f, &_v_thres); // read _v_thres; if _v_method=0, the threshold used for vector range method;
	getpara_int(f, &_recVec);	// read _recVec; whether recording output and hidden layers' vectors (1) or not (0);

	free(line); line=NULL;
	fclose(f);
}

// functions for the network training;
float euclid_dist(Real *x1, Real *x2)
{ // calculate euclidean distance between two vectors x1 and x2;
	assert(x1!=NULL); assert(x2!=NULL);
	int i;
  	float dist=0.0;
  	for(i=0;i<_pho_features;i++)
    	dist+=(x1[i]-x2[i])*(x1[i]-x2[i]);
  	return dist;
}

int vect_check(Real *x1, Real *x2)
{ // check vector's bit-based threshold between two vectors x1 and x2;
	assert(x1!=NULL); assert(x2!=NULL);
	int i, inThres=1;
	for(i=0;i<_pho_features;i++)
		{ if(fabs(x1[i]-x2[i])>=_v_thres) { inThres=0; break; }
		}
	return inThres;
}

char getphon(Real *vect, Real *trans)
{ // find phoneme matching vect; 
	assert(vect!=NULL); assert(trans!=NULL); 
	int i, j, ind, MaxDist, numInThres, *InThresSet=NULL, curind;
	char curPhon='=';
	
	switch(_v_method)
		{ case 0: // using phoneme with smallest Euclidean distance to vect as trans;
		  		ind=-1; MaxDist=1e6;
		  		for(i=0;i<_pho_number;i++)
					{ if(euclid_dist(vect,_phon[i].vec)<=MaxDist) { ind=i; MaxDist=euclid_dist(vect,_phon[i].vec); }
					}
		  		assert(ind!=-1);
		  		for(i=0;i<_pho_features;i++)
					trans[i]=_phon[ind].vec[i];
				curPhon=_phon[ind].name;
				break;		
		  case 1: // using phoneme with 0.5 threshold to set trans;
		  		numInThres=0;
		  		for(i=0;i<_pho_number;i++)
					{ if(vect_check(vect,_phon[i].vec)) numInThres++;
		  			}
				if(numInThres==0)
					{ // no such phoneme;
		  	  		  for(i=0;i<_pho_features;i++)
						trans[i]=-2.0;
					}
		  		else if(numInThres==1)
					{ // there is only one phoneme that matches this requirement;
			  		  for(i=0;i<_pho_number;i++)
						{ if(vect_check(vect,_phon[i].vec))
							{ for(j=0;j<_pho_features;j++)
								trans[j]=_phon[i].vec[j];
							  curPhon=_phon[i].name;
							  break;
							}
						}
					}
		  		else
					{ // there are more than one phoneme that match this requirement, randomly select one!
			  		  InThresSet=malloc(numInThres*sizeof(int)); assert(InThresSet!=NULL);
			  		  curind=0;
			  		  for(i=0;i<_pho_number;i++)
						{ if(vect_check(vect,_phon[i].vec)) { InThresSet[curind]=i; curind++; }
						}
			  		  ind=(int)(_Rand0_1*numInThres);
			  		  for(i=0;i<_pho_features;i++)
						trans[i]=_phon[InThresSet[ind]].vec[i];
			  		  curPhon=_phon[InThresSet[ind]].name;
					  free(InThresSet); InThresSet=NULL;
					}
				break;
		 default: break;		
		}
	return(curPhon);
}

Real cal_accu(Real *out, Real *target, char *transPhon)
{ // calculate accuracy by comparing out with target;
	assert(out!=NULL); assert(target!=NULL); 
	int i, j, same, NoAccu, cur;
	Real *vect=NULL, *trans=NULL, *transout=NULL;	// vect is each segment of out, transout is translated out based on accuMethod;
	char curPhon='=';
		
	// initialize transout
	transout=malloc(_PhonoS*sizeof(Real)); assert(transout!=NULL);
	for(i=0;i<_PhonoS;i++) 
		transout[i]=0.0;
	
	// translate phoneme by phoneme
	cur=0;
	for(i=0;i<_PhonoS;i+=_pho_features)
		{ vect=malloc(_pho_features*sizeof(Real)); assert(vect!=NULL);
		  for(j=0;j<_pho_features;j++)
			vect[j]=out[i+j];
		  trans=malloc(_pho_features*sizeof(Real)); assert(trans!=NULL);
		  for(j=0;j<_pho_features;j++)
		  	trans[j]=0.0;

		  transPhon[cur]=getphon(vect, trans); cur++;		  
		  for(j=0;j<_pho_features;j++)
			transout[i+j]=trans[j];

		  free(vect); vect=NULL; free(trans); trans=NULL;	// release memory for vect and trans;
		}
	
	// check correct translation
	NoAccu=0;
	for(i=0;i<_PhonoS;i+=_pho_features)
		{ same=1;
		  for(j=0;j<_pho_features;j++)
			{ if(transout[i+j]!=target[i+j]) { same=0; break; }
			}
		  if(same==1) NoAccu++;
		}
	free(transout); transout=NULL;	// release memory for transout;
	
	if(NoAccu/(float)(_PhonoS/(float)(_pho_features))<1.0) return 0.0;
	else return 1.0;
}

Real getaccu_PtoP(Net *net, ExampleSet *examples, int ticks)
{ // calculate accuracy of the network during ptop training;
	assert(net!=NULL); assert(examples!=NULL); assert(ticks!=0);
	int i, j;
	Example *ex=NULL;
  	Real *target=NULL, *out=NULL, accu, itemaccu, avgaccu;
	char *transPhon=NULL;	// record translated phonemes;
	
	accu=0.0;
	for(i=0;i<examples->numExamples;i++)
    	{ ex=&examples->examples[i];	// get each example;
      	  crbp_forward(net,ex);	// put to the network;
	  	
		  // initialize out and target;
		  out=malloc(_PhonoS*sizeof(Real)); assert(out!=NULL); 
		  target=malloc(_PhonoS*sizeof(Real)); assert(target!=NULL);
		  for(j=0;j<_PhonoS;j++)
			{ out[j]=output->outputs[ticks-1][j];	// get output from the network;
			  target[j]=get_value(ex->targets,output->index,ticks-1,j);	// get target from the example;
			}
		  
		  // initialize transPhon;
		  transPhon=malloc((int)(_PhonoS/(float)(_pho_features))*sizeof(char)); assert(transPhon!=NULL);
		  for(j=0;j<(int)(_PhonoS/(float)(_pho_features));j++)
		  	transPhon[j]='=';
		  		  
		  itemaccu=cal_accu(out,target,transPhon); // calculate item accuracy;
		  accu+=itemaccu;	// accumulate accuracy;

		  free(out); out=NULL; free(target); target=NULL;	// release memory for out and target;
		  free(transPhon); transPhon=NULL;	// release memory for transPhon; 
    	}
	avgaccu=accu/(float)(examples->numExamples);	// calculate average accuracy;
  	return avgaccu;
}

Real getaccu_OtoP(Net *net, ExampleSet *examples, int ticks, int iter, FILE *f, char *fName, FILE *f2, char *fName2, FILE *f3, char *fName3)
{ // calculate accuracy of the network during otop training;
	assert(net!=NULL); assert(examples!=NULL); assert(ticks!=0);
	assert(f!=NULL); assert(fName!=NULL); assert(f2!=NULL); assert(fName2!=NULL); 
	if(_recVec==1) { assert(f3!=NULL); assert(fName3!=NULL); }
	int i, j;
	Example *ex=NULL;
  	Real *target=NULL, *out=NULL, accu, itemaccu, avgaccu;
	char *transPhon=NULL;	// record translated phonemes;
	
	if((f=fopen(fName,"a+"))==NULL) { printf("Can't open %s\n", fName); exit(1); }
	fprintf(f,"%d\t%d", iter, examples->numExamples);
	if((f2=fopen(fName2,"a+"))==NULL) { printf("Can't open %s\n", fName2); exit(1); }
	fprintf(f2,"%d\t%d", iter, examples->numExamples);
	if(_recVec==1)
		{ if((f3=fopen(fName3,"a+"))==NULL) { printf("Can't open %s\n", fName3); exit(1); }
		  fprintf(f3,"%d\t%d", iter, examples->numExamples);
		}
	
	accu=0.0;
	for(i=0;i<examples->numExamples;i++)
    	{ ex=&examples->examples[i];	// get each example;
      	  crbp_forward(net,ex);	// put to the network;
	  	
		  // initialize out and target;
		  out=malloc(_PhonoS*sizeof(Real)); assert(out!=NULL); 
		  target=malloc(_PhonoS*sizeof(Real)); assert(target!=NULL);
		  for(j=0;j<_PhonoS;j++)
			{ out[j]=output->outputs[ticks-1][j];	// get output from the network;
			  target[j]=get_value(ex->targets,output->index,ticks-1,j);	// get target from the example;
			}
		  
		  // initialize transPhon;
		  transPhon=malloc((int)(_PhonoS/(float)(_pho_features))*sizeof(char)); assert(transPhon!=NULL);
		  for(j=0;j<(int)(_PhonoS/(float)(_pho_features));j++)
		  	transPhon[j]='=';
		  		  
		  itemaccu=cal_accu(out,target,transPhon);	// caculate item accuracy;
		  	
		  fprintf(f,"\t%5.3f", itemaccu);	// record item accuracy;
		  
		  // record transPhon; 
		  fprintf(f2,"\t");
		  for(j=0;j<(int)(_PhonoS/(float)(_pho_features));j++)
		  	fprintf(f2,"%c", transPhon[j]);

		  if(_recVec==1)
		  	{ // record output vector
		  	  fprintf(f3,"\t");
		  	  for(j=0;j<_PhonoS;j++)
		  		fprintf(f3,"%2.1f ", out[j]);
			}
		  
		  accu+=itemaccu;	// accumulate item accuracy;

		  free(out); out=NULL; free(target); target=NULL;	// release memory for out and target;
		  free(transPhon); transPhon=NULL;	// release memory for transPhon; 
    	}
	avgaccu=accu/(float)(examples->numExamples);	// calculate average accuracy;

	fprintf(f,"\t%5.3f\n", avgaccu); fclose(f);	
	fprintf(f2,"\n"); fclose(f2);
	if(_recVec==1) { fprintf(f3,"\n"); fclose(f3); }

  	return avgaccu;
}

void train_PtoP(Net *net, ExampleSet *TrExm_PtoP, char *weightF, int ticks, FILE *f, char *fName)
{ // training the ptop cleanup units and record training error and accuracies;
	assert(net!=NULL); assert(TrExm_PtoP!=NULL); assert(weightF!=NULL); assert(ticks!=0);
	assert(f!=NULL); assert(fName!=NULL);
	int i, iter, count;
	int ii, jj, loop, loop_out;	// for logarithm-like sampling;
	Real error, accuTr; // record error and training accuracy;
	Example *ex;

	printf("Start PtoP training!\n");

	ii=1; jj=0; loop=20; loop_out=80; // for logarithm-like sampling;
	error=0.0; count=1; accuTr=0.0;
	for(iter=1;iter<=_iter_ptop;iter++)
		{ ex=get_random_example(TrExm_PtoP);	// randomly select a training example; 
		  crbp_forward(net,ex); // feed the example to the network;
		  crbp_compute_gradients(net,ex);	// compute gradients 
		  error+=compute_error(net,ex); // accumulate training errors;
		  bptt_apply_deltas(net);	// apply deltas in back propagation;
	
		  // record status;
		  if(_samp_method==0)
			{ if(count==_rep_ptop)
				{ error=error/(float)count; // calculate average error;
				  accuTr=getaccu_PtoP(net, TrExm_PtoP, ticks); // calculate training error;
				  printf("iter=%d\terr=%5.3f\taccuTr=%5.3f\n", iter, error, accuTr);	  // display status on screen;
				  // store ptop parameters and results into f2;
				  if((f=fopen(fName,"a+"))==NULL) { printf("Can't open %s\n", fName); exit(1); }
				  fprintf(f, "%d\t%5.3f\t%5.3f\n", iter, error, accuTr); fclose(f);					  
				  error=0.0; accuTr=0.0; count=1;  // reset error, accuTr, and count;
				}
			  else count++;
			}
		  else if(_samp_method==1)
			{ if((iter!=0)&&((iter==(int)(pow(10.0,ii)+loop*pow(10.0,ii-1)*jj))||(iter%(int)(pow(10.0,ii+1))==0)))
				{ // adjust ii and jj to calculate next step
				  if(iter%(int)(pow(10.0,ii+1))==0) { ii+=1; jj=1; }
				  else
					{ if(jj*loop>=loop_out) jj=1;
					  else jj+=1;
					}
				  error=error/(float)count; // calculate average error;
				  accuTr=getaccu_PtoP(net, TrExm_PtoP, ticks); // calculate training error;
				  printf("iter=%d\terr=%5.3f\tacuTr=%5.3f\n", iter, error, accuTr);   // display status on screen;
				  // store parameters and results into f;
				  if((f=fopen(fName,"a+"))==NULL) { printf("Can't open %s\n", fName); exit(1); }
				  fprintf(f, "%d\t%5.3f\t%5.3f\n", iter, error, accuTr); fclose(f);
				  error=0.0; accuTr=0.0; count=1;  // reset error, accuTr, and count;
				}
			  else count++;
			}
		}
	save_weights(net,weightF);  // save final network weights after ptop training;

  	printf("Done PtoP training!\n");
}

void train_OtoP(Net *net, ExampleSet *TrExm, ExampleSet *TeExm, char *weightF, int ticks, FILE *f1, char *fName1, FILE *f3, char *fName3, FILE *f4, char *fName4, FILE *f5, char *fName5, FILE *f6, char *fName6, FILE *f7, char *fName7, FILE *f8, char *fName8, FILE *f9, char *fName9)
{ // train the network and record the training error and accuracies;
  	assert(net!=NULL); assert(TrExm!=NULL); assert(TeExm!=NULL); assert(weightF!=NULL); assert(ticks!=0);
	assert(f1!=NULL); assert(f3!=NULL); assert(f4!=NULL); assert(f5!=NULL); assert(f6!=NULL); assert(f7!=NULL); 
	if(_recVec==1) { assert(f8!=NULL); assert(f9!=NULL); }
	assert(fName1!=NULL); assert(fName3!=NULL); assert(fName4!=NULL); assert(fName5!=NULL); assert(fName6!=NULL); assert(fName7!=NULL);
	if(_recVec==1) { assert(fName8!=NULL); assert(fName9!=NULL); }
	int i, iter, count;
	int ii, jj, loop, loop_out;	// for logarithm-like sampling;
	int *trainAct=NULL;	// record frequency of occurrence of each example;
  	Real error, accuTr, accuTe;	// record error, training and testing accuracies;
	Example *ex;
	
  	printf("Start OtoP training!\n");
	
	// initialize trainAct;
	trainAct=malloc(TrExm->numExamples*sizeof(int)); assert(trainAct!=NULL);
	for(i=0;i<TrExm->numExamples;i++) 
		trainAct[i]=0;
	
	ii=1; jj=0; loop=20; loop_out=80; // for logarithm-like sampling;	
	error=0.0; count=1; accuTr=0.0; accuTe=0.0;
	for(iter=1;iter<=_iter;iter++)
  		{ ex=get_random_example(TrExm);	// randomly select a training example; 
		  crbp_forward(net,ex);	// feed the example to the network;
		  crbp_compute_gradients(net,ex);	// compute gradients 
		  error+=compute_error(net,ex);	// accumulate training errors;
		  bptt_apply_deltas(net);	// apply deltas in back propagation;

		  trainAct[ex->index]++;	// count the occurrence of each example during training;
		  // record status;
		  if(_samp_method==0)
		  	{ if(count==_rep)
				{ error=error/(float)count;	// calculate average error;
			  	  accuTr=getaccu_OtoP(net, TrExm, ticks, iter, f3, fName3, f6, fName6, f8, fName8);	// calculate training accuracy;
				  accuTe=getaccu_OtoP(net, TeExm, ticks, iter, f4, fName4, f7, fName7, f9, fName9);	// calculate testing accuracy;

				  printf("iter=%d\terr=%5.3f\tacuTr=%5.3f\tacuTe=%5.3f\n", iter, error, accuTr, accuTe);	// display status on screen;

				  // store parameters and results into f1;
	  			  if((f1=fopen(fName1,"a+"))==NULL) { printf("Can't open %s\n", fName1); exit(1); }
				  fprintf(f1, "%d\t%5.3f\t%5.3f\t%5.3f\n", iter, error, accuTr, accuTe); fclose(f1);
				  
				  // store trainAct into f5;
				  if((f5=fopen(fName5,"a+"))==NULL) { printf("Can't open %s\n", fName5); exit(1); }
				  fprintf(f5, "%d\t%d", iter, TrExm->numExamples);	
				  for(i=0;i<TrExm->numExamples;i++)
				  	fprintf(f5, "\t%d", trainAct[i]);
				  fprintf(f5,"\n"); fclose(f5);
				  
				  error=0.0; accuTr=0.0; accuTe=0.0; count=1;	// reset error, accuTr, accuTe, and count;
			 	}
    		  else count++;
		  	}
		  else if(_samp_method==1)
		  	{ if((iter!=0)&&((iter==(int)(pow(10.0,ii)+loop*pow(10.0,ii-1)*jj))||(iter%(int)(pow(10.0,ii+1))==0)))
				{ // adjust ii and jj to calculate next step
				  if(iter%(int)(pow(10.0,ii+1))==0) { ii+=1; jj=1; }
				  else
				  	{ if(jj*loop>=loop_out) jj=1;
					  else jj+=1;
					}
				  // record status;	
				  error=error/(float)count;	// calculate average error;
			  	  accuTr=getaccu_OtoP(net, TrExm, ticks, iter, f3, fName3, f6, fName6, f8, fName8);	// calculate training accuracy;
				  accuTe=getaccu_OtoP(net, TeExm, ticks, iter, f4, fName4, f7, fName7, f9, fName9);	// calculate testing accuracy;

				  printf("iter=%d\terr=%5.3f\tacuTr=%5.3f\tacuTe=%5.3f\n", iter, error, accuTr, accuTe);	// display status on screen;

				  // store parameters and results into f1;
	  			  if((f1=fopen(fName1,"a+"))==NULL) { printf("Can't open %s\n", fName1); exit(1); }
				  fprintf(f1, "%d\t%5.3f\t%5.3f\t%5.3f\n", iter, error, accuTr, accuTe); fclose(f1);
				  
				  // store parameters and results into f5;
				  if((f5=fopen(fName5,"a+"))==NULL) { printf("Can't open %s\n", fName5); exit(1); }
				  fprintf(f5, "%d\t%d", iter, TrExm->numExamples);	
				  for(i=0;i<TrExm->numExamples;i++)
				  	fprintf(f5, "\t%d", trainAct[i]);
				  fprintf(f5,"\n"); fclose(f5);
				  
				  error=0.0; accuTr=0.0; accuTe=0.0; count=1;	// reset error, accuTr, accuTe, and count; 
		  		}
		  	  else count++;
		  	}
  		}
  	free(trainAct); trainAct=NULL;
	save_weights(net,weightF);	// save final network weights;

	printf("Done OtoP training!\n");
}

void crtFName(char **fName, char *subDirect, char *root, char *name)
{ // create file name as root plus name;
	assert(root!=NULL); assert(name!=NULL);
	*fName=malloc((strlen(subDirect)+2+_FileLen)*sizeof(char)); assert(*fName!=NULL); 
	strcpy(*fName, root); strcat(*fName, name);
}

void initF(FILE **f, char *fName, char *format1, char *format2, int size, char *format3)
{ // initialize the first line of f;
	assert(f!=NULL); assert(fName!=NULL); assert(format1!=NULL);
	int i;
	if((*f=fopen(fName,"w+"))==NULL) { printf("Can't open %s\n", fName); exit(1); }
	fprintf(*f, format1);
	if(format2!=NULL) 
		{ for(i=0;i<size;i++)
			fprintf(*f,format2,i+1);
		}
	if(format3!=NULL) fprintf(*f, format3);
	fclose(*f);	
}

// main function;
void main(int argc,char *argv[])
{ // main function: initialize network, and train, and calculate parameters;
  	int i, iseq;
	unsigned int run;
	FILE *f=NULL, *f1=NULL, *f2=NULL, *f3=NULL, *f4=NULL, *f5=NULL, *f6=NULL, *f7=NULL, *f8=NULL, *f9=NULL;
	char sep[]="/", *seedDirect=NULL, *subDirect=NULL, *locDirect=NULL, *root=NULL;
	char *outF=NULL, *outF_PtoP=NULL, *weightF=NULL, *weightF_PtoP=NULL, *itemacuTrF=NULL, *itemacuTeF=NULL, *trainfreqF=NULL, *outPhonTrF=NULL, *outPhonTeF=NULL; 
	char *outPhonTrVecF=NULL, *outPhonTeVecF=NULL;
	
	announce_version(); setbuf(stdout,NULL); 

	readpara();	// reading network parameters and parameters for running;
	// all the parameters for running can also be set by input arguments;
	for(i=1;i<argc;i++)
		{ if(strcmp(argv[i],"-seed")==0){ assert(atoi(argv[i+1])>=0); _seed=atol(argv[i+1]); i++; }	// set random seed via '-seed' argument;
		  else if(strncmp(argv[i],"-iter",5)==0){ assert(atoi(argv[i+1])>0); _iter=atoi(argv[i+1]); i++; }	// set total number of training iterations via '-iter' argument;
		  else if(strncmp(argv[i],"-rep",4)==0){ assert(atoi(argv[i+1])>0); _rep=atoi(argv[i+1]); i++; }	// set sampling point during training via '-rep' argument;
		  else if(strncmp(argv[i],"-ptop",5)==0){ assert((atoi(argv[i+1])==0)||(atoi(argv[i+1])==1)); _ptop=atoi(argv[i+1]); i++; }	// set whether involve ptop training (1) or not (0) via '-ptop' argument;
		  else if(strncmp(argv[i],"-iter_ptop",10)==0){ assert(atoi(argv[i+1])>0); _iter_ptop=atoi(argv[i+1]); i++; } // set total number of ptop training iteractions via '-iter_ptop' argument;
		  else if(strncmp(argv[i],"-rep_ptop",9)==0){ assert(atoi(argv[i+1])>0); _rep_ptop=atoi(argv[i+1]); i++; }	// set sampling point during ptop training via '-rep_ptop' argument;
		  else if(strncmp(argv[i],"-samp",5)==0){ assert((atoi(argv[i+1])==0)||(atoi(argv[i+1])==1)); _samp_method=atoi(argv[i+1]); i++; }	// set sampling method via '-samp' argument;
		  else if(strncmp(argv[i],"-met",4)==0) { assert((atoi(argv[i+1])==0)||(atoi(argv[i+1])==1)); _v_method=atoi(argv[i+1]); i++; }	// set accuracy calculation method via '-met' argument;
		  else if(strncmp(argv[i],"-thres",6)==0) { assert((atof(argv[i+1])>=0.0)||(atof(argv[i+1])<=1.0)); _v_thres=atof(argv[i+1]); i++; }	// set accuracy calculation threshold for vector based method via '-thres' argument;
		  else if(strncmp(argv[i],"-recvec",7)==0) { assert((atoi(argv[i+1])==0)||(atoi(argv[i+1])==1)); _recVec=atoi(argv[i+1]); i++; }	// set recording vector output via '-recvec' argument;
		}

	printf("input subdic name(int): "); scanf("%d", &iseq); printf("subdic is %d\n", iseq);
	if(_seed==0) _seed=(long)(time(NULL))+100*iseq; // if the input seed is 0, meaning randomly setting the seed;

	// handle subDirect, locDirect;
	subDirect=malloc((strlen("./")+2+(int)(log10((double)(iseq))+1)+1)*sizeof(char)); assert(subDirect!=NULL);
	locDirect=malloc(((int)(log10((double)(iseq))+1)+1)*sizeof(char)); assert(locDirect!=NULL);
	sprintf(locDirect, "%d", iseq); strcpy(subDirect, "./"); strcat(subDirect, locDirect); strcat(subDirect, sep);
	if(mkdir(subDirect, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)==-1) { printf("can't create directory %s!\n", subDirect); exit(1); } // used in Linux;

	// save seed into seed.txt;
	seedDirect=malloc((strlen(subDirect)+strlen("seed.txt")+1)*sizeof(char)); assert(seedDirect!=NULL);
	strcpy(seedDirect, subDirect); strcat(seedDirect, "seed.txt");
	if((f=fopen(seedDirect,"w"))==NULL) { printf("Can't create %s\n", seedDirect); exit(1); } fprintf(f, "Seed=%u\n", _seed); fclose(f);	// store seed into seed.txt;
	free(seedDirect); seedDirect=NULL; f=NULL;

	// set up directories;
	root=malloc((strlen(subDirect)+2+_FileLen)*sizeof(char)); assert(root!=NULL); strcpy(root, subDirect);

	mikenet_set_seed(_seed); printf("Seed = %u\n", _seed);	// print out the seed used;

	// 1) build network;
	if(_ptop==1) build_reading_model(_tick_PtoP); // build a network, with TIME number of time ticks;
	else build_reading_model(_tick_OtoP);
	printf("No. Conns: %d\n", count_connections(reading));	// calculate number of connections and print out;

	// 2) build phoneme dictionary, training and testing examples;
	load_phoneme(_phoF);  // initialize phoneme;
	train_exm=load_examples(_exTrF, _tick_OtoP); test_exm=load_examples(_exTeF, _tick_OtoP);	// load training and testing examples;
	if(_ptop==1) train_ptop_exm=load_examples(_exTrF_PtoP, _tick_PtoP);	// load training example for ptop training;

	// crete result file names;
	crtFName(&outF, subDirect, root, "output.txt");	// record training error, training accuracy and testing accuracy;
	if(_ptop==1) crtFName(&outF_PtoP, subDirect, root, "output_ptop.txt");
	crtFName(&itemacuTrF, subDirect, root, "itemacu_tr.txt"); crtFName(&itemacuTeF, subDirect, root, "itemacu_te.txt");	// record item-based training and testing accuracy;
	crtFName(&weightF, subDirect, root, "weights.txt");	// record connection weights of the network;
	if(_ptop==1) crtFName(&weightF_PtoP, subDirect, root, "weights_ptop.txt");	// record connection weights of the ptop network;
	crtFName(&trainfreqF, subDirect, root, "trainfreq.txt"); 	// record accumulative occurring frequency of training examples during training;
	crtFName(&outPhonTrF, subDirect, root, "outphonTr.txt"); crtFName(&outPhonTeF, subDirect, root, "outphonTe.txt");	// record output phonemes of training and testing examples;
	if(_recVec==1) { crtFName(&outPhonTrVecF, subDirect, root, "outphonTrVec.txt"); crtFName(&outPhonTeVecF, subDirect, root, "outphonTeVec.txt"); }	// record output phonological vector values of training and testing examples;

	// create result files' headers
	initF(&f1, outF, "ITER\tErr\tAcuTr\tAcuTe\n", NULL, 0, NULL);
	if(_ptop==1) initF(&f2, outF_PtoP, "ITER\tErr\tAcuTr\n", NULL, 0, NULL);
	initF(&f3, itemacuTrF, "ITER\tNoItem", "\tAcu%d", train_exm->numExamples, "\tAvg\n");
	initF(&f4, itemacuTeF, "ITER\tNoItem", "\tAcu%d", test_exm->numExamples, "\tAvg\n");
	initF(&f5, trainfreqF, "ITER\tNoItem", "\tF%d", train_exm->numExamples, "\n");
	initF(&f6, outPhonTrF, "ITER\tNoItem", "\tPhon%d", train_exm->numExamples, "\n");
	initF(&f7, outPhonTeF, "ITER\tNoItem", "\tPhon%d", test_exm->numExamples, "\n");
	if(_recVec==1)
		{ initF(&f8, outPhonTrVecF, "ITER\tNoItem", "\tPV%d", train_exm->numExamples, "\n");
		  initF(&f9, outPhonTeVecF, "ITER\tNoItem", "\tPV%d", test_exm->numExamples, "\n");
		}
	
	// 3) train the network;
	printf("Start training!\n");
	if(_ptop==1) 
		{ train_PtoP(reading, train_ptop_exm, weightF_PtoP, _tick_PtoP, f2, outF_PtoP);
		  //build_reading_model(_tick_OtoP);	// if during PtoP training, a different tick is used, one needs to rebuild network;
		  load_weights(reading,weightF_PtoP);	// load weights after ptop training;
		  printf("c3: %f\n", c3->weights[2][5]);
		}
	train_OtoP(reading, train_exm, test_exm, weightF, _tick_OtoP, f1, outF, f3, itemacuTrF, f4, itemacuTeF, f5, trainfreqF, f6, outPhonTrF, f7, outPhonTeF, f8, outPhonTrVecF, f9, outPhonTeVecF);	// train network;
	printf("Done!\n");

	// 4) release memory for the training and testing examples, and phoneme dictionary;
	free(train_exm); train_exm=NULL;	// free training_examples;
	free(test_exm); test_exm=NULL;	// free testing_examples;
	if(_ptop==1) { free(train_ptop_exm); train_ptop_exm=NULL; }	// free training_ptop_examples;
	delete_phoneme();	// empty _phon;

	// 5) release memory for the network;
	free_net(reading); reading=NULL;

	// free result file names;
	free(outF); outF=NULL; f1=NULL;
	if(_ptop==1) { free(outF_PtoP); outF_PtoP=NULL; f2=NULL; }
	free(weightF); weightF=NULL;
	if(_ptop==1) { free(weightF_PtoP); weightF_PtoP=NULL; }
	free(itemacuTrF); itemacuTrF=NULL; f3=NULL; free(itemacuTeF); itemacuTeF=NULL; f4=NULL; 
	free(trainfreqF); trainfreqF=NULL; f5=NULL;
	free(outPhonTrF); outPhonTrF=NULL; f6=NULL; free(outPhonTeF); outPhonTeF=NULL; f7=NULL; 
	if(_recVec==1) { free(outPhonTrVecF); outPhonTrVecF=NULL; f8=NULL; free(outPhonTeVecF); outPhonTeVecF=NULL; f9=NULL; }

	free(root); root=NULL; free(subDirect); subDirect=NULL; free(locDirect); locDirect=NULL;				
}

