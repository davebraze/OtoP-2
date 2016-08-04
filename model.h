// network components;
extern Net *reading;
extern Group *input,*hidden,*output,*phohid;
extern Connections *c1,*c2,*c3,*c4,*c5;
extern ExampleSet *train_exm, *test_exm;
// parameters for network;
#define TIME 7
#define TAI 1
#define EPSI 1e-3
#define INTCONST 0.25
#define ACTTYPE TANH_ACTIVATION
#define ERRRAD 0.1
#define RANGE 0.1
#define OrthoS 208
#define HidS 100
#define PhonoS 55
#define PhoHidS 20
// function for building the network;
void build_model(void);
