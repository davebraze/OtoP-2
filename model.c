#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <mikenet/simulator.h>

#include "model.h"

Net *reading;
Group *input,*hidden,*output,*phohid;
Connections *c1,*c2,*c3,*c4,*c5;
ExampleSet *reading_examples;

void build_model(void)
{ // build a network, with TIME number of time ticks
	int i;
  
  	default_tai=TAI;
  	reading=create_net(TIME);
  	reading->integrationConstant=INTCONST;

  	/* learning rate */
  	default_epsilon=EPSI;
  	default_activationType=ACTTYPE;

  	/* error radius */
  	default_errorRadius=ERRRAD;

  	/* create our groups. format is: name, num of units,  ticks */
  	input=init_group("Ortho",OrthoS,TIME);
  	hidden=init_group("Hidden",HidS,TIME);
  	output=init_group("Phono",PhonoS,TIME);
  	phohid=init_group("PhoHid",PhoHidS,TIME);

  	/* now add our groups to the network object */
  	bind_group_to_net(reading,input);
  	bind_group_to_net(reading,hidden);
  	bind_group_to_net(reading,output);
  	bind_group_to_net(reading,phohid);

  	/* now connect our groups, instantiating connection objects c1 through c4 */
  	c1=connect_groups(input,hidden);
  	c2=connect_groups(hidden,output);
  	c3=connect_groups(output,output);
  	c4=connect_groups(output,phohid);
  	c5=connect_groups(phohid,output);

  	/* add connections to our network */
  	bind_connection_to_net(reading,c1);
  	bind_connection_to_net(reading,c2);
  	bind_connection_to_net(reading,c3);
  	bind_connection_to_net(reading,c4);
  	bind_connection_to_net(reading,c5);

  	/* randomize the weights in the connection objects. Second argument is weight range. */
  	randomize_connections(c1,RANGE);
  	randomize_connections(c2,RANGE);
  	randomize_connections(c3,RANGE);
  	randomize_connections(c4,RANGE);
  	randomize_connections(c5,RANGE);

  	c3->epsilon=EPSI; 
	c4->epsilon=EPSI; 
	c5->epsilon=EPSI;

  	precompute_topology(reading,input);
  	for(i=0;i<reading->numGroups;i++)
  		printf("%s %d\n",reading->groups[i]->name,reading->groups[i]->whenDataLive);
  
  	for(i=0;i<c3->to->numUnits;i++)
    	{ // freeze c3 connection weights to 0.75!
      	  c3->weights[i][i]=0.75;	 
      	  c3->frozen[i][i]=1;
    	}
}
