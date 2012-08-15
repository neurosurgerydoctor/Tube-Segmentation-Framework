#include "histogram-pyramids.hpp"
#include <cmath>
#include <iostream>
using namespace cl;

HistogramPyramid2D::HistogramPyramid2D(OpenCL ocl) {
    this->ocl = ocl;
}
HistogramPyramid3D::HistogramPyramid3D(OpenCL ocl) {
    this->ocl = ocl;
}

int HistogramPyramid::getSum() {
    return this->sum;
}

void HistogramPyramid3D::create(Image3D baseLevel, int sizeX, int sizeY, int sizeZ) {
    // Make baseLevel into power of 2 in all dimensions
    if(sizeX == sizeY && sizeY == sizeZ && log2(sizeX) == round(log2(sizeX))) {
        size = sizeX;
    }else{
        // Find largest size and find closest power of two
        int largestSize = std::max(sizeX, std::max(sizeY, sizeZ));
        int i = 1;
        while(pow(2, i) < largestSize)
            i++;
        size = pow(2, i);
    }
    std::cout << "3D HP size: " << size << std::endl;

    // Create all levels
    HPlevels.push_back(baseLevel);
    int levelSize = size / 2;
    HPlevels.push_back(Image3D(
                ocl.context,
                CL_MEM_READ_WRITE,
                ImageFormat(CL_R, CL_UNSIGNED_INT8),
                levelSize, levelSize, levelSize
    ));
    levelSize /= 2;
    HPlevels.push_back(Image3D(
                ocl.context,
                CL_MEM_READ_WRITE,
                ImageFormat(CL_R, CL_UNSIGNED_INT8),
                levelSize, levelSize, levelSize
    ));
    levelSize /= 2;
    // 16 bit
    HPlevels.push_back(Image3D(
                ocl.context,
                CL_MEM_READ_WRITE,
                ImageFormat(CL_R, CL_UNSIGNED_INT16),
                levelSize, levelSize, levelSize
    ));
    levelSize /= 2;
    HPlevels.push_back(Image3D(
                ocl.context,
                CL_MEM_READ_WRITE,
                ImageFormat(CL_R, CL_UNSIGNED_INT16),
                levelSize, levelSize, levelSize
    ));
    levelSize /= 2;

    // The rest will use 32 bit
    for(int i = 5; i < log2(size); i++) {
        HPlevels.push_back(Image3D(
                    ocl.context,
                    CL_MEM_READ_WRITE,
                    ImageFormat(CL_R, CL_UNSIGNED_INT32),
                    levelSize, levelSize, levelSize
        ));
        levelSize /= 2;
    }

    // Do construction iterations
    Kernel constructHPLevelKernel(ocl.program, "constructHPLevel3D");
    levelSize = size;
    for(int i = 0; i < log2((float)size)-1; i++) {
        constructHPLevelKernel.setArg(0, HPlevels[i]);
        constructHPLevelKernel.setArg(1, HPlevels[i+1]);
        levelSize /= 2;
        ocl.queue.enqueueNDRangeKernel(
            constructHPLevelKernel,
            NullRange,
            NDRange(levelSize, levelSize, levelSize),
            NullRange
        );
    }

    // Get total sum and return it
    int * sum = new int[8];
    cl::size_t<3> offset;
    offset[0] = 0;
    offset[1] = 0;
    offset[2] = 0;
    cl::size_t<3> region;
    region[0] = 2;
    region[1] = 2;
    region[2] = 2;
    ocl.queue.enqueueReadImage(HPlevels[HPlevels.size()-1], CL_TRUE, offset, region, 0, 0, sum);
    this->sum = sum[0] + sum[1] + sum[2] + sum[3] + sum[4] + sum[5] + sum[6] + sum[7];
}

void HistogramPyramid2D::create(Image2D baseLevel, int sizeX, int sizeY) {
    // Make baseLevel into power of 2 in all dimensions
    if(sizeX == sizeY && log2(sizeX) == round(log2(sizeX))) {
        size = sizeX;
    } else {
        // Find largest size and find closest power of two
        int largestSize = std::max(sizeX, sizeY);
        int i = 1;
        while(pow(2, i) < largestSize)
            i++;
        size = pow(2, i);
    }
    std::cout << "2D HP size: " << size << std::endl;

    // Create all levels
    HPlevels.push_back(baseLevel);
    int levelSize = size / 2;
    HPlevels.push_back(Image2D(
                ocl.context,
                CL_MEM_READ_WRITE,
                ImageFormat(CL_R, CL_UNSIGNED_INT8),
                levelSize, levelSize
    ));
    levelSize /= 2;
    HPlevels.push_back(Image2D(
                ocl.context,
                CL_MEM_READ_WRITE,
                ImageFormat(CL_R, CL_UNSIGNED_INT8),
                levelSize, levelSize
    ));
    levelSize /= 2;
    // 16 bit
    HPlevels.push_back(Image2D(
                ocl.context,
                CL_MEM_READ_WRITE,
                ImageFormat(CL_R, CL_UNSIGNED_INT16),
                levelSize, levelSize
    ));
    levelSize /= 2;
    HPlevels.push_back(Image2D(
                ocl.context,
                CL_MEM_READ_WRITE,
                ImageFormat(CL_R, CL_UNSIGNED_INT16),
                levelSize, levelSize
    ));
    levelSize /= 2;

    // The rest will use 32 bit
    for(int i = 5; i < log2(size); i++) {
        HPlevels.push_back(Image2D(
                    ocl.context,
                    CL_MEM_READ_WRITE,
                    ImageFormat(CL_R, CL_UNSIGNED_INT32),
                    levelSize, levelSize
        ));
        levelSize /= 2;
    }

    // Do construction iterations
    Kernel constructHPLevelKernel(ocl.program, "constructHPLevel2D");
    levelSize = size;
    for(int i = 0; i < log2((float)size)-1; i++) {
        constructHPLevelKernel.setArg(0, HPlevels[i]);
        constructHPLevelKernel.setArg(1, HPlevels[i+1]);
        levelSize /= 2;
        ocl.queue.enqueueNDRangeKernel(
            constructHPLevelKernel,
            NullRange,
            NDRange(levelSize, levelSize),
            NullRange
        );
    }

    // Get total sum and return it
    int * sum = new int[4];
    cl::size_t<3> offset;
    offset[0] = 0;
    offset[1] = 0;
    offset[2] = 0;
    cl::size_t<3> region;
    region[0] = 2;
    region[1] = 2;
    region[2] = 1;
    ocl.queue.enqueueReadImage(HPlevels[HPlevels.size()-1], CL_TRUE, offset, region, 0, 0, sum);
    this->sum = sum[0] + sum[1] + sum[2] + sum[3];
}

void HistogramPyramid2D::traverse(Kernel kernel, int arguments) {
    for(int i = 0; i < 11; i++) {
        int l = i;
        if(i >= HPlevels.size())
            // if not using all levels, just add the last levels as dummy arguments
            l = HPlevels.size()-1;
        kernel.setArg(i+arguments, HPlevels[l]);
    }

    int global_work_size = sum + 64 - (sum - 64*(sum / 64));
    ocl.queue.enqueueNDRangeKernel(kernel, NullRange, NDRange(global_work_size), NDRange(64));
}

void HistogramPyramid3D::traverse(Kernel kernel, int arguments) {
    kernel.setArg(arguments, this->size);
    kernel.setArg(arguments+1, this->sum);
    for(int i = 0; i < 10; i++) {
        int l = i;
        if(i >= HPlevels.size())
            // if not using all levels, just add the last levels as dummy arguments
            l = HPlevels.size()-1;
        kernel.setArg(i+arguments+2, HPlevels[l]);
    }

    int global_work_size = sum + 64 - (sum - 64*(sum / 64));
    ocl.queue.enqueueNDRangeKernel(kernel, NullRange, NDRange(global_work_size), NDRange(64));
}

void HistogramPyramid3D::update() {
    // Do construction iterations
    Kernel constructHPLevelKernel(ocl.program, "constructHPLevel3D");
    int levelSize = size;
    for(int i = 0; i < log2((float)size)-1; i++) {
        constructHPLevelKernel.setArg(0, HPlevels[i]);
        constructHPLevelKernel.setArg(1, HPlevels[i+1]);
        levelSize /= 2;
        ocl.queue.enqueueNDRangeKernel(
            constructHPLevelKernel,
            NullRange,
            NDRange(levelSize, levelSize, levelSize),
            NullRange
        );
    }

    // Get total sum and return it
    int * sum = new int[8];
    cl::size_t<3> offset;
    offset[0] = 0;
    offset[1] = 0;
    offset[2] = 0;
    cl::size_t<3> region;
    region[0] = 2;
    region[1] = 2;
    region[2] = 2;
    ocl.queue.enqueueReadImage(HPlevels[HPlevels.size()-1], CL_TRUE, offset, region, 0, 0, sum);
    this->sum = sum[0] + sum[1] + sum[2] + sum[3] + sum[4] + sum[5] + sum[6] + sum[7];
}

void HistogramPyramid3D::update(Image3D newBaseLevel, int sizeX, int sizeY, int sizeZ) {
    cl::size_t<3> offset;
    offset[0] = 0;
    offset[1] = 0;
    offset[2] = 0;
    cl::size_t<3> region;
    region[0] = sizeX;
    region[1] = sizeY;
    region[2] = sizeZ;
    ocl.queue.enqueueCopyImage(newBaseLevel, HPlevels[0], offset, offset, region);
    this->update();
}

Buffer HistogramPyramid2D::createPositionBuffer() {
    Buffer * positions = new Buffer(
            ocl.context,
            CL_MEM_READ_WRITE,
            2*sizeof(int)*sum
    );
    Kernel kernel(ocl.program, "createPositions2D");
    kernel.setArg(0, (*positions));
    kernel.setArg(1, this->size);
    kernel.setArg(2, this->sum);
    this->traverse(kernel, 3);
    return *positions;
}

Buffer HistogramPyramid3D::createPositionBuffer() {
    Buffer * positions = new Buffer(
            ocl.context,
            CL_MEM_READ_WRITE,
            3*sizeof(int)*sum
    );
    Kernel kernel(ocl.program, "createPositions3D");
    kernel.setArg(0, (*positions));
    this->traverse(kernel, 1);
    return *positions;
}

std::string insertHPOpenCLCode(std::string source, int size) {
}
