/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   FieldPoint.h
 * Author: jwhh9
 *
 * Created on June 3, 2018, 11:33 AM
 */


// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2014 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2014 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2014 Mateusz Loskot, London, UK.
// Copyright (c) 2014 Adam Wulkiewicz, Lodz, Poland.

// This file was modified by Oracle on 2014.
// Modifications copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRIES_FIELDPOINT_H
#define BOOST_GEOMETRY_GEOMETRIES_FIELDPOINT_H

#include <cstddef>
#include <cstdlib>
#include <memory>
#include <array>
#include <utility>
#include <iostream>  
#include <iterator>
#include <algorithm>

#include <boost/mpl/int.hpp>
#include <boost/static_assert.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/coordinate_system.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>

#include "constants.h"
#include "redismodule.h"
#include "Module/ParseUtil.h"

#include "Module/SaveUtil.h"
#include "Module/LoadUtil.h"
#include "Module/ParseUtil.h"



namespace boost { namespace geometry
{

// Silence warning C4127: conditional expression is constant
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4127)
#endif

    
namespace model
{

template
<
    typename CoordinateType,
    std::size_t DimensionCount,
    typename CoordinateSystem,
    std::size_t FieldLength
>
class FieldPoint
{
private:
    // The following enum is used to fully instantiate the
    // CoordinateSystem class and check the correctness of the units
    // passed for non-Cartesian coordinate systems.
    enum { cs_check = sizeof(CoordinateSystem) };

public:
    typedef std::array<CoordinateType, DimensionCount> Dim_Ary;
    typedef std::array<CoordinateType, FieldLength> Fld_Ary;
    
    inline FieldPoint(){}

    /// @brief Constructor to set three values
    inline FieldPoint(Dim_Ary &coords, Fld_Ary &storage)
    {
        m_values = coords;
        m_storage = storage;
    }
    
    constexpr static std::size_t ReqSize()
    {
        return ARG_COUNT_MIN + DimensionCount + FieldLength;
    }
    
    static void save(RedisModuleIO *rdb, void* value)
    {
        FieldPoint *pPoint = (FieldPoint*) value;
        
        SaveUtil::save(rdb, pPoint->m_values);
        SaveUtil::save(rdb, pPoint->m_storage);
    }
    
    static void* load(RedisModuleIO *rdb, int encver)
    {
        
        FieldPoint *pPoint = new FieldPoint();
        
        LoadUtil::load(rdb, pPoint->m_values);
        LoadUtil::load(rdb, pPoint->m_storage);
        
        return pPoint;        
    }
    
    std::stringstream& operator<<(std::stringstream& ofs)
    {
        auto iter_out = std::ostream_iterator<CoordinateType>(ofs, " ");
        std::copy(m_values.begin(), m_values.end(), iter_out);
        
        ofs << " --> ";
        
        std::copy(m_storage.begin(), m_storage.end(), iter_out);
        return ofs;
    }
    
    static void parse(RedisModuleCtx *ctx, RedisModuleString **argv, int argc, FieldPoint *pPt)
    {
        ParseUtil::parse(pPt->m_values, argv, ARG_COUNT_MIN);
        ParseUtil::parse(pPt->m_storage, argv, ARG_COUNT_MIN + DimensionCount);
    }
    
    Fld_Ary getStorage()
    {
        return m_storage;
    }
    

    /// @brief Get a coordinate
    /// @tparam K coordinate to get
    /// @return the coordinate
    template <std::size_t K>
    inline CoordinateType const& get() const
    {
        BOOST_STATIC_ASSERT(K < DimensionCount);
        return m_values[K];
    }

    /// @brief Set a coordinate
    /// @tparam K coordinate to set
    /// @param value value to set
    template <std::size_t K>
    inline void set(CoordinateType const& value)
    {
        BOOST_STATIC_ASSERT(K < DimensionCount);
        m_values[K] = value;
    }

private:

    Dim_Ary m_values;
    Fld_Ary m_storage;
};


} // namespace model

// Adapt the point to the concept
#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{
template
<
    typename CoordinateType,
    std::size_t DimensionCount,
    typename CoordinateSystem,
    std::size_t FieldLength
>
struct tag<model::FieldPoint<CoordinateType, DimensionCount, CoordinateSystem, FieldLength> >
{
    typedef point_tag type;
};

template
<
    typename CoordinateType,
    std::size_t DimensionCount,
    typename CoordinateSystem,
    std::size_t FieldLength
>
struct coordinate_type<model::FieldPoint<CoordinateType, DimensionCount, CoordinateSystem, FieldLength> >
{
    typedef CoordinateType type;
};

template
<
    typename CoordinateType,
    std::size_t DimensionCount,
    typename CoordinateSystem,
    std::size_t FieldLength
>
struct coordinate_system<model::FieldPoint<CoordinateType, DimensionCount, CoordinateSystem, FieldLength> >
{
    typedef CoordinateSystem type;
};

template
<
    typename CoordinateType,
    std::size_t DimensionCount,
    typename CoordinateSystem,
    std::size_t FieldLength
>
struct dimension<model::FieldPoint<CoordinateType, DimensionCount, CoordinateSystem, FieldLength> >
    : boost::mpl::int_<DimensionCount>
{};

template
<
    typename CoordinateType,
    std::size_t DimensionCount,
    typename CoordinateSystem,
    std::size_t FieldLength,
    std::size_t Dimension
>
struct access<model::FieldPoint<CoordinateType, DimensionCount, CoordinateSystem, FieldLength>, Dimension>
{
    static inline CoordinateType get(
        model::FieldPoint<CoordinateType, DimensionCount, CoordinateSystem, FieldLength> const& p)
    {
        return p.template get<Dimension>();
    }

    static inline void set(
        model::FieldPoint<CoordinateType, DimensionCount, CoordinateSystem, FieldLength>& p,
        CoordinateType const& value)
    {
        p.template set<Dimension>(value);
    }
};

} // namespace traits
#endif // DOXYGEN_NO_TRAITS_SPECIALIZATIONS

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_GEOMETRIES_FIELDPOINT_H


