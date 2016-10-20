#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mikenet/simulator.h>

#include "model.h"

// function to build up the model
void build_PtoP(int ticks)
{ // build a network, with TIME number of time ticks
	assert(ticks!=0);

	/* tai*/
  	default_tai=_tai;
	
	/* create network */
	PtoP=create_net(ticks); 

	/* integration constant */
	PtoP->integrationConstant=_intconst;
	
	/* learning rate, activation pattern, error method, error level, and error radius */
	default_epsilon=_epsi;
  	default_activationType=_acttype;
	default_errorComputation=_errortype;
	default_weightNoiseType=_weightnoisetype;
	default_weightNoise=_weightnoise;
	default_activationNoise=_actnoise;
	default_inputNoise=_inputnoise;
	default_errorRadius=_errrad;

	/* create our groups. format is: name, num of units,  ticks */
  	input_ptop=init_group("Ortho",_OrthoS,ticks);
  	hidden_ptop=init_group("Hidden",_HidS,ticks);
  	output_ptop=init_group("Phono",_PhonoS,ticks);
  	phohid_ptop=init_group("PhoHid",_PhoHidS,ticks);

	/* now add our groups to the network object */
  	bind_group_to_net(PtoP,input_ptop);
  	bind_group_to_net(PtoP,hidden_ptop);
  	bind_group_to_net(PtoP,output_ptop);
  	bind_group_to_net(PtoP,phohid_ptop);

	/* now connect our groups, instantiating connection objects c1 through c4 */
  	c1_ptop=connect_groups(input_ptop,hidden_ptop);
  	c2_ptop=connect_groups(hidden_ptop,output_ptop);
  	c3_ptop=connect_groups(output_ptop,output_ptop);
  	c4_ptop=connect_groups(output_ptop,phohid_ptop);
  	c5_ptop=connect_groups(phohid_ptop,output_ptop);

	/* add connections to our network */
  	bind_connection_to_net(PtoP,c1_ptop);
  	bind_connection_to_net(PtoP,c2_ptop);
  	bind_connection_to_net(PtoP,c3_ptop);
  	bind_connection_to_net(PtoP,c4_ptop);
  	bind_connection_to_net(PtoP,c5_ptop);

	/* randomize the weights in the connection objects. Second argument is weight range. */
  	randomize_connections(c1_ptop,_range);
  	randomize_connections(c2_ptop,_range);
  	randomize_connections(c3_ptop,_range);
  	randomize_connections(c4_ptop,_range);
  	randomize_connections(c5_ptop,_range);

  	c3_ptop->epsilon=_epsi; 
	c4_ptop->epsilon=_epsi; 
	c5_ptop->epsilon=_epsi;

	int i;
  	precompute_topology(PtoP,input_ptop);
  	for(i=0;i<PtoP->numGroups;i++)
  		printf("%s %d\n",PtoP->groups[i]->name,PtoP->groups[i]->whenDataLive);  
  	for(i=0;i<c3_ptop->to->numUnits;i++)
    	{ c3_ptop->weights[i][i]=0.75; c3_ptop->frozen[i][i]=1;	// freeze c3 connection weights to 0.75!
    	}
}

void build_OtoP(int ticks)
{ // build a network, with TIME number of time ticks
	assert(ticks!=0);
		
  	/* tai */
  	default_tai=_tai;

	/* create network */
	OtoP=create_net(ticks); 
	/* integration constant */
	OtoP->integrationConstant=_intconst;

	/* learning rate, activation pattern, error method, error level, and error radius */
	default_epsilon=_epsi;
  	default_activationType=_acttype;
	default_errorComputation=_errortype;
	default_weightNoiseType=_weightnoisetype;
	default_weightNoise=_weightnoise;
	default_activationNoise=_actnoise;
	default_inputNoise=_inputnoise;
	default_errorRadius=_errrad;

	/* create our groups. format is: name, num of units,  ticks */
  	input=init_group("Ortho",_OrthoS,ticks);
  	hidden=init_group("Hidden",_HidS,ticks);
  	output=init_group("Phono",_PhonoS,ticks);
  	phohid=init_group("PhoHid",_PhoHidS,ticks);

	/* now add our groups to the network object */
  	bind_group_to_net(OtoP,input);
  	bind_group_to_net(OtoP,hidden);
  	bind_group_to_net(OtoP,output);
  	bind_group_to_net(OtoP,phohid);

	/* now connect our groups, instantiating connection objects c1 through c4 */
  	c1=connect_groups(input,hidden);
  	c2=connect_groups(hidden,output);
  	c3=connect_groups(output,output);
  	c4=connect_groups(output,phohid);
  	c5=connect_groups(phohid,output);

	/* add connections to our network */
  	bind_connection_to_net(OtoP,c1);
  	bind_connection_to_net(OtoP,c2);
  	bind_connection_to_net(OtoP,c3);
  	bind_connection_to_net(OtoP,c4);
  	bind_connection_to_net(OtoP,c5);

	/* randomize the weights in the connection objects. Second argument is weight range. */
  	randomize_connections(c1,_range);
  	randomize_connections(c2,_range);
  	randomize_connections(c3,_range);
  	randomize_connections(c4,_range);
  	randomize_connections(c5,_range);

  	c3->epsilon=_epsi; 
	c4->epsilon=_epsi; 
	c5->epsilon=_epsi;

	int i;
  	precompute_topology(OtoP,input);
  	for(i=0;i<OtoP->numGroups;i++)
  		printf("%s %d\n",OtoP->groups[i]->name,OtoP->groups[i]->whenDataLive);  
  	for(i=0;i<c3->to->numUnits;i++)
    	{ c3->weights[i][i]=0.75; c3->frozen[i][i]=1;	// freeze c3 connection weights to 0.75!
    	}
}

int count_connections(Net *net)
{ // calculate number of connections in the network;
	assert(net!=NULL);
  	int i, j, k, count=0;
  	for(i=0;i<net->numConnections;i++)
    	count += (net->connections[i]->from->numUnits)*(net->connections[i]->to->numUnits);
  	return count;
}

void load_phoneme(char *PhoF)
{ // initialize phon by reading from PhoF;
	assert(PhoF!=NULL);
	int curphon, curvec;
	FILE *f=NULL;
	char line[_LineLen], *p=NULL;
	// initialize PhoDic;
	_phon=malloc(_pho_number*sizeof(Phoneme)); assert(_phon!=NULL);
	// read from PhoF;
	if((f=fopen(PhoF,"r"))==NULL){ printf("Can't open %s\n", PhoF); exit(1); }
	curphon=0;
	while(!feof(f))
    	{ fgets(line, _LineLen, f);
		  _phon[curphon].name=strtok(line," ")[0];
		  _phon[curphon].vec=malloc(_pho_features*sizeof(Real)); assert(_phon[curphon].vec!=NULL);
		  curvec=0;
		  while(p=strtok(NULL, " "))
			{ _phon[curphon].vec[curvec]=atof(p); 
			  curvec++;
			}
		  curphon++;
		}
	fclose(f);
	/*
	// print PhoDic
	int i, j;
	for(i=0;i<_pho_number;i++)
		{ printf("Phoneme %c: ", _phon[i].name);
		  for(j=0;j<_pho_features;j++)
	  		printf("%2.1f ", _phon[i].vec[j]);
	     	  printf("\n");
		}
	*/
}

void delete_phoneme(void)
{ // delete phon;
	int i;
	for(i=0;i<_pho_number;i++)
		{ free(_phon[i].vec); _phon[i].vec=NULL;
		}
	free(_phon); _phon=NULL;
}
