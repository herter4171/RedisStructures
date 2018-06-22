/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   kd_test.cpp
 * Author: jwhh9
 *
 * Created on June 22, 2018, 3:55 AM
 */

#include <stdlib.h>
#include <iostream>
#include <random>
#include <cmath>

#include <boost/timer/timer.hpp>

#include <ANN/ANN.h>

#include "constants.h"
/*
 * Simple C++ Test Suite
 */

std::default_random_engine generator;
std::uniform_real_distribution<double> dist;

int result_count = 1; // number of nearest neighbors

// Allocate result containers
ANNidxArray nnIdx = new ANNidx[result_count];
ANNdistArray dists = new ANNdist[result_count];    


ANNpoint getPoint()
{
    ANNpoint pt = new ANNcoord[POINT_DIMENSIONS];

    for (int j = 0; j < POINT_DIMENSIONS; j++)
    {
        pt[j] = dist(generator);             
        //std::cout << pt[j] << " ";
    }
    
    //std::cout << std::endl;
    
    return pt;
}

ANNpointArray getPointArray(int nPts)
{
    ANNpointArray ptAry = new ANNpoint[nPts];
    
    //std::cout << "PT ARY:\n----------------------\n";
    
    for (int i = 0; i < nPts; i++)
    {        
        ptAry[i] = getPoint();
    }   
    
    //std::cout << "----------------------------\n\n";
    
    return ptAry;
}

void testQuery(int nPts)
{
    // Set point tree
    ANNpointArray dataPts = getPointArray(nPts);
    ANNkd_tree kdTree(dataPts, nPts, POINT_DIMENSIONS);
    
    // Set query points
    ANNpointArray queryPts = getPointArray(nPts);
    
    boost::timer::auto_cpu_timer t;
    
    for (int i = 0; i < nPts; i++)
    {    
        
        kdTree.annkSearch(queryPts[i], result_count, nnIdx, dists, 0);   

        //std::cout << "NN: Index Distance\n";
        for (int j = 0; j < result_count; j++) 
        {
            dists[j] = sqrt(dists[j]);
            //std::cout << j << " " << nnIdx[j] << " " << dists[j] << "\n";
        }
    }
}

int main(int argc, char** argv)
{
    int nPts = 1E6;
    dist = std::uniform_real_distribution<double> (0.0, 1.0);
    
    testQuery(nPts);
    
    std::cout << "\n\nWOO\n\n";
}

