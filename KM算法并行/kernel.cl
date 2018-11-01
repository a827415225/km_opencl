#pragma OPENCL EXTENSION cl_amd_printf : enable
__kernel void update(int nx,  int ny, __global double* d,__global int *visx ,__global int *visy ,__global double *slack,__global double *lx,__global double *ly)
{
	int i=get_global_id(0);
	if(i<nx)
	{
		if (visx[i])  lx[i] -= d[0];
	}
	if(i>=nx)  //�޸Ķ����Ҫ�����в��ڽ������е�Y�����slackֵ����ȥd
	{
		if (visy[i-nx])
				ly[i-nx] += d[0];
		else
				slack[i-nx] -= d[0];
	}
	
	
}
