#pragma once

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstddef>

const std::size_t ARG_COUNT_MIN = 2; // Key/val at minimum
const std::size_t POINT_DIMENSIONS = 3; // 3D points

// Key/val and 3 coordiantes
const std::size_t POINT_QUERY_LENGTH = ARG_COUNT_MIN + POINT_DIMENSIONS;

const std::size_t SCALAR_FIELD_LENGTH = 1;
#endif