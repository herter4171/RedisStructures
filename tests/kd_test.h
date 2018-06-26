/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   kd_test.h
 * Author: jwhh9
 *
 * Created on June 22, 2018, 10:20 AM
 */

#ifndef KD_TEST_H
#define KD_TEST_H

#include <stdlib.h>
#include <iostream>
#include <random>
#include <cmath>
#include <vector>
#include <array>

#include <boost/timer/timer.hpp>
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>

#include <ANN/ANN.h>

#include "constants.h"
#include "FieldPoint.h"

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

std::default_random_engine generator;
std::uniform_real_distribution<double> dist;

typedef bg::model::FieldPoint<double, POINT_DIMENSIONS, bg::cs::cartesian, std::array<double, 1>> scalar_pt;

int result_count = 1; // number of nearest neighbors

// Allocate result containers
ANNidxArray nnIdx = new ANNidx[result_count];
ANNdistArray dists = new ANNdist[result_count];    

scalar_pt getPoint();

template<std::size_t L>
std::array<scalar_pt, L> getPointArray()
{
    std::array<scalar_pt, L> ptAry;    
    
    for (int i = 0; i < L; i++)
    {        
        ptAry[i] = getPoint();
    }       
    
    return ptAry;
}

template<std::size_t L>
void testQuery()
{
    // Set point tree
    std::array<scalar_pt, L> dataPts = getPointArray<L>();
    ANNkd_tree kdTree(dataPts, L, POINT_DIMENSIONS);
    
    // Set query points
    std::array<scalar_pt, L> queryPts = getPointArray<L>();
    
    boost::timer::auto_cpu_timer t;
    
    for (int i = 0; i < L; i++)
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


#endif /* KD_TEST_H */

