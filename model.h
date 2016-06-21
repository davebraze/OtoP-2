#define TIME 7
// network components;
extern Net *reading;
extern Group *input,*hidden,*output,*phohid;
extern ExampleSet *reading_examples;
extern Connections *c1,*c2,*c3,*c4,*c5;
// parameters for network;
#define TAI 1
#define EPSILON 0.001
#define INTECONST 0.25
#define ACTTYPE TANH_ACTIVATION
#define ERRORRAD 0.1
#define RANDRANGE 0.1

// function for building the network;
void build_model(void);
void free_model(void);
