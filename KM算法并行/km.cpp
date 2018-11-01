#include "km.h"

using namespace std;

AssignmentProblemSolver::AssignmentProblemSolver(int nx,int ny,double **w)
{
	this->nx = nx;
	
	this->ny = ny;
	
	visx = new int [nx];

	visy=new int [ny];

	link=new int [ny];

	lx=new double[nx];

	ly=new double[ny];

	slack=new double[ny];
	cost = new double*[nx];

	for (int i = 0; i < nx; i++)
	{
		cost[i] = new double[ny];
	}
	for (int i = 0; i < nx;i++)
	for (int j = 0; j < ny; j++)
		cost[i][j] = w[i][j];
	GpuInitial();

}

AssignmentProblemSolver::~AssignmentProblemSolver()
{
}
int AssignmentProblemSolver:: DFS(int x)
{
	visx[x] = 1;
	for (int y = 0; y < ny; y++)
	{
		if (visy[y])
			continue;
		double t = lx[x] + ly[y] - cost[x][y];
		if (abs(t) <= 1e-6)       //
		{
			visy[y] = 1;
			if (link[y] == -1 || DFS(link[y]))
			{
				link[y] = x;
				return 1;
			}
		}
		else if (slack[y] > t)  //���������ͼ��slack ȡ��С�ġ�//slack[y]��ʾ������xֵ��lx[x]�����٣�x��y�������ϡ�
			slack[y] = t;
	}
	return 0;

}
void AssignmentProblemSolver::update(double d)
{
errNum=	clEnqueueWriteBuffer(commandQueue, memObjects[0], CL_TRUE,



		0, 1 * sizeof(double), &d,



		0, NULL, NULL);

	
errNum|=clEnqueueWriteBuffer(commandQueue, memObjects[1], CL_TRUE,



		0, nx * sizeof(int), visx,



		0, NULL, NULL);

errNum|=clEnqueueWriteBuffer(commandQueue, memObjects[2], CL_TRUE,



		0, ny* sizeof(int), visy,



		0, NULL, NULL);
	
errNum|=clEnqueueWriteBuffer(commandQueue, memObjects[3], CL_TRUE,



		0, ny* sizeof(double), slack,



		0, NULL, NULL);

errNum|=clEnqueueWriteBuffer(commandQueue, memObjects[4], CL_TRUE,



		0, nx* sizeof(double), lx,



		0, NULL, NULL);

errNum|=clEnqueueWriteBuffer(commandQueue, memObjects[5], CL_TRUE,



		0, ny* sizeof(double), ly,



		0, NULL, NULL);

	
errNum |= clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL,



		globalWorkSize, NULL,



		0, NULL, NULL);


// ���� ��ȡִ�н�����ͷ�OpenCL��Դ

errNum |= clEnqueueReadBuffer(commandQueue, memObjects[3], CL_TRUE,



		0, ny* sizeof(double), slack,



		0, NULL, NULL);
//for (int i = 0; i < ny; i++)
	//cout << slack[i] << endl;

errNum |= clEnqueueReadBuffer(commandQueue, memObjects[4], CL_TRUE,



	0, nx* sizeof(double), lx,



	0, NULL, NULL);

errNum |= clEnqueueReadBuffer(commandQueue, memObjects[5], CL_TRUE,



	0, ny* sizeof(double), ly,



	0, NULL, NULL);


/*for (int i = 0; i < nx; i++)
	if (visx[i])
		lx[i] -= d;
	for (int i = 0; i < ny; i++)  //�޸Ķ����Ҫ�����в��ڽ������е�Y�����slackֵ����ȥd
	{
		if (visy[i])
			ly[i] += d;
		else
			slack[i] -= d;
	}*/


}
double AssignmentProblemSolver::solve()
{
	double start = static_cast<double>(cvGetTickCount());
	int i, j;
	memset(link, -1, ny*sizeof(int));
	memset(ly, 0, ny*sizeof(double));
	for (i = 0; i < nx; i++)            //lx��ʼ��Ϊ����������������
	for (j = 0, lx[i] = -DBL_MAX; j < ny; j++)
	if (cost[i][j] > lx[i])
		lx[i] = cost[i][j];
	for (int x = 0; x < nx; x++)
	{
		for (i = 0; i < ny; i++)
			slack[i] = DBL_MAX;
		while (1)
		{
			memset(visx, 0, nx*sizeof(int));
			memset(visy, 0, ny*sizeof(int));
			if (DFS(x))     //���ɹ����ҵ�������죩����õ�������ɣ�������һ���������
				break;  //��ʧ�ܣ�û���ҵ�����죩������Ҫ�ı�һЩ��ı�ţ�ʹ��ͼ�п��бߵ��������ӡ�
			//����Ϊ����������������У���������������б���������X����ı��ȫ����ȥһ������d��
			//������������е�Y����ı��ȫ������һ������d
			double d = DBL_MAX;
			for (i = 0; i < ny; i++)
			if (!visy[i] && d > slack[i])
				d = slack[i];
			update(d);
		}
	}
	double res = 0;
	for (i = 0; i < ny; i++)
	if (link[i] > -1)
		res += cost[link[i]][i];
	double time = ((double)cvGetTickCount() - start) / cvGetTickFrequency();
	cout << "������ʱ��Ϊ:" << time << "us" << endl;
	return res;
}
cl_int AssignmentProblemSolver::ConvertToString(const char *pFileName, std::string &Str)

{

	size_t		uiSize = 0;

	size_t		uiFileSize = 0;

	char		*pStr = NULL;

	std::fstream fFile(pFileName, (std::fstream::in | std::fstream::binary));

	if (fFile.is_open())

	{

		fFile.seekg(0, std::fstream::end);

		uiSize = uiFileSize = (size_t)fFile.tellg();  // ����ļ���С

		fFile.seekg(0, std::fstream::beg);

		pStr = new char[uiSize + 1];

		if (NULL == pStr)

		{

			fFile.close();

			return 0;

		}

		fFile.read(pStr, uiFileSize);				// ��ȡuiFileSize�ֽ�

		fFile.close();

		pStr[uiSize] = '\0';

		Str = pStr;

		delete[] pStr;

		return 0;

}

//cout << "Error: Failed to open cl file\n:" << pFileName << endl;

return -1;

}
cl_context AssignmentProblemSolver::CreateContext()
{
	cl_int errNum;

	cl_uint numPlatforms;

	cl_platform_id firstPlatformId;

	cl_context context = NULL;



	//ѡ����õ�ƽ̨�еĵ�һ��

	errNum = clGetPlatformIDs(1, &firstPlatformId, &numPlatforms);

	if (errNum != CL_SUCCESS || numPlatforms <= 0)

	{

		std::cerr << "Failed to find any OpenCL platforms." << std::endl;

		return NULL;

	}

	//����һ��OpenCL�����Ļ���

	cl_context_properties contextProperties[] =

	{

		CL_CONTEXT_PLATFORM,

		(cl_context_properties)firstPlatformId,

		0

	};

	context = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_GPU,

		NULL, NULL, &errNum);



	return context;

}
cl_command_queue AssignmentProblemSolver::CreateCommandQueue(cl_context context, cl_device_id *device)

{

	cl_int errNum;

	cl_device_id *devices;

	cl_command_queue commandQueue = NULL;

	size_t deviceBufferSize = -1;

	// ��ȡ�豸��������С

	errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &deviceBufferSize);

	if (deviceBufferSize <= 0)

	{

		std::cerr << "No devices available.";

		return NULL;

	}


	// Ϊ�豸���仺��ռ�

	devices = new cl_device_id[deviceBufferSize / sizeof(cl_device_id)];

	errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, deviceBufferSize, devices, NULL);

	//ѡȡ�����豸�еĵ�һ��

	commandQueue = clCreateCommandQueue(context, devices[0], 0, NULL);

	*device = devices[0];

	delete[] devices;

	return commandQueue;

}
cl_program AssignmentProblemSolver::CreateProgram(cl_context context, cl_device_id device, const char* fileName)

{

	cl_int errNum;

	cl_program program;

	std::ifstream kernelFile(fileName, std::ios::in);

	if (!kernelFile.is_open())

	{

		std::cerr << "Failed to open file for reading: " << fileName << std::endl;

		return NULL;

	}

	std::ostringstream oss;

	oss << kernelFile.rdbuf();

	std::string srcStdStr = oss.str();

	const char *srcStr = srcStdStr.c_str();

	program = clCreateProgramWithSource(context, 1,

		(const char**)&srcStr,

		NULL, NULL);

	errNum = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);

	return program;

}
bool AssignmentProblemSolver::CreateMemObjects(cl_context context, cl_mem memObjects[6])

{
	// ���������ڴ����
	memObjects[0] = clCreateBuffer(context,

		CL_MEM_READ_WRITE,  // �����ڴ�Ϊֻ���������Դ��������ڴ渴�Ƶ��豸�ڴ�

		nx* sizeof(int),		  // �����ڴ�ռ��С

		NULL,

		NULL);

	memObjects[1] = clCreateBuffer(context,

		CL_MEM_READ_WRITE,  // �����ڴ�Ϊֻ���������Դ��������ڴ渴�Ƶ��豸�ڴ�

		ny * sizeof(int),		  // �����ڴ�ռ��С

		NULL,

		NULL);

	memObjects[2] = clCreateBuffer(context,

		CL_MEM_READ_WRITE,  // �����ڴ�Ϊֻ���������Դ��������ڴ渴�Ƶ��豸�ڴ�

		nx * sizeof(double),		  // �����ڴ�ռ��С

		NULL,

		NULL);

	memObjects[3] = clCreateBuffer(context,

		CL_MEM_READ_WRITE,  // �����ڴ�Ϊֻ���������Դ��������ڴ渴�Ƶ��豸�ڴ�

		1 * sizeof(double),		  // �����ڴ�ռ��С

		NULL,

		NULL);

	memObjects[4] = clCreateBuffer(context,

		CL_MEM_READ_WRITE,  // �����ڴ�Ϊֻ���������Դ��������ڴ渴�Ƶ��豸�ڴ�

		nx * sizeof(double),		  // �����ڴ�ռ��С

		NULL,

		NULL);

	memObjects[5] = clCreateBuffer(context,

		CL_MEM_READ_WRITE,  // �����ڴ�Ϊֻ���������Դ��������ڴ渴�Ƶ��豸�ڴ�

		ny * sizeof(double),		  // �����ڴ�ռ��С

		NULL,

		NULL);

	if ((NULL == memObjects[0]) || (NULL == memObjects[1]) || (NULL == memObjects[2]) || NULL == memObjects[3] || NULL == memObjects[4] || NULL == memObjects[5])
	{
		cout << "Error creating memory objects" << endl;

		return false;
	}
	return true;
	
}
void Cleanup(cl_context context, cl_command_queue commandQueue,

	cl_program program, cl_kernel kernel, cl_mem memObjects[6])

{

	for (int i = 0; i < 6; i++)

	{

		if (memObjects[i] != 0)

			clReleaseMemObject(memObjects[i]);

	}

	if (commandQueue != 0)

		clReleaseCommandQueue(commandQueue);



	if (kernel != 0)

		clReleaseKernel(kernel);



	if (program != 0)

		clReleaseProgram(program);



	if (context != 0)

		clReleaseContext(context);
	return;
}
void AssignmentProblemSolver::GpuInitial()
{
	// һ��ѡ��OpenCLƽ̨������һ��������

	context = CreateContext();

	// ���� �����豸�������������

	commandQueue = CreateCommandQueue(context, &device);

	CreateMemObjects(context, memObjects);

	//���������͹����������

	program = CreateProgram(context, device, "kernel.cl");

	kernel = clCreateKernel(program, "update", NULL);

	// �ġ� ����OpenCL�ں˲������ڴ�ռ�
	

	errNum = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)(&nx));

	errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)(&ny));

	errNum |= clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&memObjects[0]);

	errNum |= clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&memObjects[1]);

	errNum |= clSetKernelArg(kernel, 4, sizeof(cl_mem), (void *)&memObjects[2]);

	errNum |= clSetKernelArg(kernel, 5, sizeof(cl_mem), (void *)&memObjects[3]);

	errNum |= clSetKernelArg(kernel, 6, sizeof(cl_mem), (void *)&memObjects[4]);

	errNum |= clSetKernelArg(kernel, 7, sizeof(cl_mem), (void *)&memObjects[5]);


	if (CL_SUCCESS != errNum)
	{
		cout << "Error setting kernel arguments" << endl;
	}
	// --------------------------10.�����ں�---------------------------------

	globalWorkSize[0] = nx + ny;

}
/*
// --------------------------------------------------------------------------
// Usage example
// --------------------------------------------------------------------------
void main(void)
{
// Matrix size
int N=8; // tracks
int M=9; // detects
// Random numbers generator initialization
srand (time(NULL));
// Distance matrix N-th track to M-th detect.
vector< vector<double> > Cost(N,vector<double>(M));
// Fill matrix with random values
for(int i=0; i<N; i++)
{
for(int j=0; j<M; j++)
{
Cost[i][j] = (double)(rand()%1000)/1000.0;
std::cout << Cost[i][j] << "\t";
}
std::cout << std::endl;
}

AssignmentProblemSolver APS;

vector<int> Assignment;

cout << APS.Solve(Cost,Assignment) << endl;

// Output the result
for(int x=0; x<N; x++)
{
std::cout << x << ":" << Assignment[x] << "\t";
}

getchar();
}
*/
// --------------------------------------------------------------------------