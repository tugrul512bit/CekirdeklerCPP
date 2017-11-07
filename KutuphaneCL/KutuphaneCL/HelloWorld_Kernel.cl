#define ELE_TYPE unsigned int

uint w_hash(uint seed)  
{  
     seed = (seed ^ 61) ^ (seed >> 16);  
     seed *= 9;  
     seed = seed ^ (seed >> 4);  
     seed *= 0x27d4eb2d;  
     seed = seed ^ (seed >> 15);  
     return seed;  
}

unsigned int w_rnd_direct(__global unsigned int * intSeeds, int id)                
{
    //uint maxint=0;
    //maxint--;
    uint rndint=w_hash(intSeeds[id]);
    intSeeds[id]=rndint;
	return rndint;
}

unsigned int w_rnd_atomic(__global unsigned int * intSeeds, int id)                
{
    //uint maxint=0;
    //maxint--;
	//mem_fence(CLK_GLOBAL_MEM_FENCE);
	uint rndint=w_hash(intSeeds[id]);
	//mem_fence(CLK_GLOBAL_MEM_FENCE);
	//uint tmp0=atomic_add(&intSeeds[id],0);  
	//mem_fence(CLK_GLOBAL_MEM_FENCE);
	//atomic_sub(&intSeeds[id],tmp0);  
	//mem_fence(CLK_GLOBAL_MEM_FENCE);
	//atomic_add(&intSeeds[id],rndint);
	//mem_fence(CLK_GLOBAL_MEM_FENCE);
	atomic_xchg(&intSeeds[id],rndint);
	return rndint;
}


void kernel testkernel(global ELE_TYPE *intSeeds) 
{
	int id = get_global_id(0);
	unsigned int maxint=0; 
	maxint--;

	//w_rnd_direct(intSeeds, id);

	//mem_fence(CLK_GLOBAL_MEM_FENCE);
	unsigned int result=w_rnd_atomic(intSeeds, id);
	//mem_fence(CLK_GLOBAL_MEM_FENCE);
	if(result<maxint/2)
	{
		w_rnd_atomic(intSeeds, id);
		//mem_fence(CLK_GLOBAL_MEM_FENCE);
		w_rnd_atomic(intSeeds, id);
		//mem_fence(CLK_GLOBAL_MEM_FENCE);
	}

}