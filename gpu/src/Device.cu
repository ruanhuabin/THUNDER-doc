/***********************************************************************
 * FileName: Device.cu
 * Author  : Kunpeng WANG
 * Version :
 * Description:
 *
 * History :
 *
 **********************************************************************/
#include "Device.cuh"

// deprecated
__device__ double atomicAdd_(double* address, const double val)
{
    unsigned long long int* address_as_ull =
                            (unsigned long long int*)address;
    unsigned long long int old = *address_as_ull, assumed;
    
    do {
        assumed = old;
        old = atomicCAS(address_as_ull,
                        assumed,
                        __double_as_longlong(val + 
                                             __longlong_as_double(assumed)));
        // Note: uses integer comparison to avoid hang
        // in case of NaN (since NaN != NaN)
    } while (assumed != old);
    return __longlong_as_double(old);
}
