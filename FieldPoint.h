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
#include <boost/mpl/int.hpp>
#include <boost/static_assert.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/coordinate_system.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>

#include "redismodule.h"
#include "Deleter.h"
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
    typename Storage
>
class FieldPoint
{
private:
    // The following enum is used to fully instantiate the
    // CoordinateSystem class and check the correctness of the units
    // passed for non-Cartesian coordinate systems.
    enum { cs_check = sizeof(CoordinateSystem) };

public:
    typedef std::shared_ptr<CoordinateType> ary_ptr; 
    

    /// @brief Constructor to set three values
    inline FieldPoint(std::array<double, DimensionCount> &coords, Storage &storage)
    {
        m_values = coords;
        m_storage = storage;
    }
    
    Storage getStorage()
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

    std::array<double, DimensionCount> m_values;
    Storage m_storage;
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
    typename Storage
>
struct tag<model::FieldPoint<CoordinateType, DimensionCount, CoordinateSystem, Storage> >
{
    typedef point_tag type;
};

template
<
    typename CoordinateType,
    std::size_t DimensionCount,
    typename CoordinateSystem,
    typename Storage
>
struct coordinate_type<model::FieldPoint<CoordinateType, DimensionCount, CoordinateSystem, Storage> >
{
    typedef CoordinateType type;
};

template
<
    typename CoordinateType,
    std::size_t DimensionCount,
    typename CoordinateSystem,
    typename Storage
>
struct coordinate_system<model::FieldPoint<CoordinateType, DimensionCount, CoordinateSystem, Storage> >
{
    typedef CoordinateSystem type;
};

template
<
    typename CoordinateType,
    std::size_t DimensionCount,
    typename CoordinateSystem,
    typename Storage
>
struct dimension<model::FieldPoint<CoordinateType, DimensionCount, CoordinateSystem, Storage> >
    : boost::mpl::int_<DimensionCount>
{};

template
<
    typename CoordinateType,
    std::size_t DimensionCount,
    typename CoordinateSystem,
    typename Storage,
    std::size_t Dimension
>
struct access<model::FieldPoint<CoordinateType, DimensionCount, CoordinateSystem, Storage>, Dimension>
{
    static inline CoordinateType get(
        model::FieldPoint<CoordinateType, DimensionCount, CoordinateSystem, Storage> const& p)
    {
        return p.template get<Dimension>();
    }

    static inline void set(
        model::FieldPoint<CoordinateType, DimensionCount, CoordinateSystem, Storage>& p,
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


