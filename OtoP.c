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
unsigned int _rep=1e3;
unsigned int _iter=5e4;
int _samp_method=0;	// linear (0) or logarithm-like (1) sampling;
int _v_method=0;	// based on Euclidean-distance (0) or vector range (1);
double _v_thres=0.5;	// threshold for vector range method;
int _recVec=0;	// whether recording output and hidden layers' vectors (1) or not (0);

// functions;
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

Real getaccu(Net *net, ExampleSet *examples, int iter, FILE *f, char *fName, FILE *f2, char *fName2, FILE *f3, char *fName3)
{ // calculate accuracy of the network;
	assert(net!=NULL); assert(examples!=NULL); assert(f!=NULL); assert(fName!=NULL); assert(f2!=NULL); assert(fName2!=NULL); 
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
			{ out[j]=output->outputs[_tick-1][j];	// get output from the network;
			  target[j]=get_value(ex->targets,output->index,_tick-1,j);	// get target from the example;
			}
		  // initialize transPhon;
		  transPhon=malloc((int)(_PhonoS/(float)(_pho_features))*sizeof(char)); assert(transPhon!=NULL);
		  for(j=0;j<(int)(_PhonoS/(float)(_pho_features));j++)
		  	transPhon[j]='=';
		  		  
		  itemaccu=cal_accu(out,target,transPhon);
		  	
		  fprintf(f,"\t%5.3f", itemaccu);	// record accuracy;		  
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
		  
		  accu+=itemaccu;	// calculate accuracy;

		  free(out); out=NULL; free(target); target=NULL;	// release memory for out and target;
		  free(transPhon); transPhon=NULL;	// release memory for transPhon; 
    	}
	avgaccu=accu/(float)(examples->numExamples);

	fprintf(f,"\t%5.3f\n", avgaccu); fclose(f);	
	fprintf(f2,"\n"); fclose(f2);
	if(_recVec==1) { fprintf(f3,"\n"); fclose(f3); }

  	return avgaccu;
}

void train(Net *net, ExampleSet *TrExm, ExampleSet *TeExm, FILE *f1, char *fName1, FILE *f2, char *fName2, FILE *f3, char *fName3, char *weightF, FILE *f4, char *fName4, FILE *f5, char *fName5, FILE *f6, char *fName6, FILE *f7, char *fName7, FILE *f8, char *fName8)
{ // train the network and record the training error and accuracies;
  	assert(net!=NULL); assert(TrExm!=NULL); assert(TeExm!=NULL); 
	assert(f1!=NULL); assert(f2!=NULL); assert(f3!=NULL); assert(f4!=NULL); assert(f5!=NULL); assert(f6!=NULL); 
	if(_recVec==1) { assert(f7!=NULL); assert(f8!=NULL); }
	assert(weightF!=NULL); 
	assert(fName1!=NULL); assert(fName2!=NULL); assert(fName3!=NULL); assert(fName4!=NULL); assert(fName5!=NULL); assert(fName6!=NULL); 
	if(_recVec==1) { assert(fName7!=NULL); assert(fName8!=NULL); }
	int i, iter, count;
	int ii, jj, loop, loop_out;
	int *trainAct=NULL;
  	Example *ex;
  	Real error, accuTr=0.0, accuTe=0.0;

	trainAct=malloc(TrExm->numExamples*sizeof(int)); assert(trainAct!=NULL);
	for(i=0;i<TrExm->numExamples;i++)
		trainAct[i]=0;
	
	ii=1; jj=0; loop=20; loop_out=80; // for logarithm-like sampling;
	error=0.0; count=1;
  	for(iter=1;iter<=_iter;iter++)
  		{ ex=get_random_example(TrExm); 
		  crbp_forward(net,ex); 
		  crbp_compute_gradients(net,ex); 
		  error+=compute_error(net,ex); 
		  bptt_apply_deltas(net);

		  trainAct[ex->index]++;	// count the occurrence of each example during training;

		  if(_samp_method==0)
		  	{ if(count==_rep)
				{ // record status;
				  error=error/(float)count;
			  	  accuTr=getaccu(net, TrExm, iter, f2, fName2, f5, fName5, f7, fName7); 
				  accuTe=getaccu(net, TeExm, iter, f3, fName3, f6, fName6, f8, fName8);

				  printf("iter=%d\terr=%5.3f\tacuTr=%5.3f\tacuTe=%5.3f\n", iter, error, accuTr, accuTe);	// display on screen;
	  			  if((f1=fopen(fName1,"a+"))==NULL) { printf("Can't open %s\n", fName1); exit(1); }
				  fprintf(f1, "%d\t%5.3f\t%5.3f\t%5.3f\n", iter, error, accuTr, accuTe);	// store parameters and results into f1;
				  fclose(f1);				  

				  error=0.0; accuTr=0.0; accuTe=0.0;
				  count=1;	// reset count;

				  if((f4=fopen(fName4,"a+"))==NULL) { printf("Can't open %s\n", fName4); exit(1); }
				  fprintf(f4, "%d\t%d", iter, TrExm->numExamples);	// store parameters and results into f4;
				  for(i=0;i<TrExm->numExamples;i++)
				  	fprintf(f4, "\t%d", trainAct[i]);
				  fprintf(f4,"\n"); fclose(f4);
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
				  error=error/(float)count;
			  	  accuTr=getaccu(net, TrExm, iter, f2, fName2, f5, fName5, f7, fName7); 
				  accuTe=getaccu(net, TeExm, iter, f3, fName3, f6, fName6, f8, fName8);
				  printf("iter=%d\terr=%5.3f\tacuTr=%5.3f\tacuTe=%5.3f\n", iter, error, accuTr, accuTe);	// display on screen;
	  			  if((f1=fopen(fName1,"a+"))==NULL) { printf("Can't open %s\n", fName1); exit(1); }
				  fprintf(f1, "%d\t%5.3f\t%5.3f\t%5.3f\n", iter, error, accuTr, accuTe);	// store parameters and results into f;
				  fclose(f1);
				  
				  error=0.0; accuTr=0.0; accuTe=0.0;
				  count=1;	// reset count; 

				  if((f4=fopen(fName4,"a+"))==NULL) { printf("Can't open %s\n", fName4); exit(1); }
				  fprintf(f4, "%d\t%d", iter, TrExm->numExamples);	// store parameters and results into f4;
				  for(i=0;i<TrExm->numExamples;i++)
				  	fprintf(f4, "\t%d", trainAct[i]);
				  fprintf(f4,"\n"); fclose(f4);
		  		}
		  	  else count++;
		  	}
  		}
  	save_weights(net,weightF);

	free(trainAct); trainAct=NULL;
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
	FILE *f=NULL, *f1=NULL, *f2=NULL, *f3=NULL, *f4=NULL, *f5=NULL, *f6=NULL, *f7=NULL, *f8=NULL;
	char sep[]="/", *seedDirect=NULL, *subDirect=NULL, *locDirect=NULL, *root=NULL;
	char *outF=NULL, *weightF=NULL, *itemacuTrF=NULL, *itemacuTeF=NULL, *trainfreqF=NULL, *outPhonTrF=NULL, *outPhonTeF=NULL; 
	char *outPhonTrVecF=NULL, *outPhonTeVecF=NULL;
	
	printf("input subdic name(int): "); scanf("%d", &iseq); printf("subdic is %d\n", iseq);
	_seed=(long)(time(NULL))+100*iseq;

	announce_version(); setbuf(stdout,NULL); 
		  
	// all the parameters can also be set by input arguments;
	for(i=1;i<argc;i++)
		{ if(strcmp(argv[i],"-seed")==0){ assert(atoi(argv[i+1])>=0); _seed=atol(argv[i+1]); i++; }	// set random seed via '-seed' argument;
		  else if(strncmp(argv[i],"-iter",5)==0){ assert(atoi(argv[i+1])>0); _iter=atoi(argv[i+1]); i++; }	// set total number of training iterations via '-iter' argument;
		  else if(strncmp(argv[i],"-rep",4)==0){ assert(atoi(argv[i+1])>0); _rep=atoi(argv[i+1]); i++; }	// set sampling point via '-rep' argument;
		  else if(strncmp(argv[i],"-samp",5)==0){ assert((atoi(argv[i+1])==0)||(atoi(argv[i+1])==1)); _samp_method=atoi(argv[i+1]); i++; }	// set sampling method via '-samp' argument;
		  else if(strncmp(argv[i],"-met",4)==0) { assert((atoi(argv[i+1])==0)||(atoi(argv[i+1])==1)); _v_method=atoi(argv[i+1]); i++; }	// set accuracy calculation method via '-met' argument;
		  else if(strncmp(argv[i],"-thres",6)==0) { assert((atof(argv[i+1])>=0.0)||(atof(argv[i+1])<=1.0)); _v_thres=atof(argv[i+1]); i++; }	// set accuracy calculation threshold for vector based method via '-thres' argument;
		  else if(strncmp(argv[i],"-recvec",7)==0) { assert((atoi(argv[i+1])==0)||(atoi(argv[i+1])==1)); _recVec=atoi(argv[i+1]); i++; }	// set recording vector output via '-recvec' argument;
		}	
	mikenet_set_seed(_seed); build_model();	// build a network, with TIME number of time ticks; 
	printf("No. Connections: %d\n", count_connections());	// calculate number of connections and print out;
	printf("Seed = %u\n", _seed);	// print out the seed used;

	load_phoneme(_phoF);  // initialize phoneme;
	train_exm=load_examples(_exTrF, _tick);	test_exm=load_examples(_exTeF, _tick);	// load training and testing examples;

	// handle subDirect, locDirect;
	subDirect=malloc((strlen("./")+2+(int)(log10((double)(iseq))+1)+1)*sizeof(char)); assert(subDirect!=NULL);
	locDirect=malloc(((int)(log10((double)(iseq))+1)+1)*sizeof(char)); assert(locDirect!=NULL);
	sprintf(locDirect, "%d", iseq); strcpy(subDirect, "./"); strcat(subDirect, locDirect); strcat(subDirect, sep);
	if(mkdir(subDirect, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)==-1) { printf("can't create directory %s!\n", subDirect); exit(1); } // used in Linux;

	// save seed into seed.txt;
	seedDirect=malloc((strlen(subDirect)+strlen("seed.txt")+1)*sizeof(char)); assert(seedDirect!=NULL);
	strcpy(seedDirect, subDirect); strcat(seedDirect, "seed.txt");
	if((f=fopen(seedDirect,"w"))==NULL) { printf("Can't create %s\n", seedDirect); exit(1); } fprintf(f, "Seed=%u\n", _seed); fclose(f);	// store seed into seed.txt;
	free(seedDirect); seedDirect=NULL;

	// set up directories for result files;
	root=malloc((strlen(subDirect)+2+_FileLen)*sizeof(char)); assert(root!=NULL); strcpy(root, subDirect);
	crtFName(&outF, subDirect, root, "output.txt");	// record training error, training accuracy and testing accuracy;
	crtFName(&itemacuTrF, subDirect, root, "itemacu_tr.txt"); crtFName(&itemacuTeF, subDirect, root, "itemacu_te.txt");	// record item-based training and testing accuracy;
	crtFName(&weightF, subDirect, root, "weights.txt");	// record connection weights of the network;
	crtFName(&trainfreqF, subDirect, root, "trainfreq.txt"); 	// record accumulative occurring frequency of training examples during training;
	crtFName(&outPhonTrF, subDirect, root, "outphonTr.txt"); crtFName(&outPhonTeF, subDirect, root, "outphonTe.txt");	// record output phonemes of training and testing examples;
	if(_recVec==1) { crtFName(&outPhonTrVecF, subDirect, root, "outphonTrVec.txt"); crtFName(&outPhonTeVecF, subDirect, root, "outphonTeVec.txt"); }	// record output phonological vector values of training and testing examples;
	// create result files' headers
	initF(&f1, outF, "ITER\tErr\tAcuTr\tAcuTe\n", NULL, 0, NULL);
	initF(&f2, itemacuTrF, "ITER\tNoItem", "\tAcu%d", train_exm->numExamples, "\tAvg\n");
	initF(&f3, itemacuTeF, "ITER\tNoItem", "\tAcu%d", test_exm->numExamples, "\tAvg\n");
	initF(&f4, trainfreqF, "ITER\tNoItem", "\tF%d", train_exm->numExamples, "\n");
	initF(&f5, outPhonTrF, "ITER\tNoItem", "\tPhon%d", train_exm->numExamples, "\n");
	initF(&f6, outPhonTeF, "ITER\tNoItem", "\tPhon%d", test_exm->numExamples, "\n");
	if(_recVec==1)
		{ initF(&f7, outPhonTrVecF, "ITER\tNoItem", "\tPV%d", train_exm->numExamples, "\n");
		  initF(&f8, outPhonTeVecF, "ITER\tNoItem", "\tPV%d", test_exm->numExamples, "\n");
		}
	printf("Start training!\n");
	train(reading, train_exm, test_exm, f1, outF, f2, itemacuTrF, f3, itemacuTeF, weightF, f4, trainfreqF, f5, outPhonTrF, f6, outPhonTeF, f7, outPhonTrVecF, f8, outPhonTeVecF);	// train network;
	printf("Done!\n");

	free(outF); outF=NULL; free(weightF); weightF=NULL; 
	free(itemacuTrF); itemacuTrF=NULL; free(itemacuTeF); itemacuTeF=NULL; 
	free(trainfreqF); trainfreqF=NULL; free(outPhonTrF); outPhonTrF=NULL; free(outPhonTeF); outPhonTeF=NULL; 
	if(_recVec==1) { free(outPhonTrVecF); outPhonTrVecF=NULL; free(outPhonTeVecF); outPhonTeVecF=NULL; }
	free(root); root=NULL; free(subDirect); subDirect=NULL; free(locDirect); locDirect=NULL;				
	
	free(train_exm); train_exm=NULL;	// free training_examples;
	free(test_exm); test_exm=NULL;	// free testing_examples;
	delete_phoneme();	// empty _phon;
	free_net(reading); reading=NULL; // free network components;
}

