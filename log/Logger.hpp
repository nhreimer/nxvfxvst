/*
 * Copyright (C) 2025 Nicholas Reimer <nicholas.hans@gmail.com>
 *
 * This file is part of a project licensed under the GNU Affero General Public License v3.0,
 * with an additional non-commercial use restriction.
 *
 * You may redistribute and/or modify this file under the terms of the GNU AGPLv3 as
 * published by the Free Software Foundation, provided that your use is strictly non-commercial.
 *
 * This software is provided "as-is", without any warranty of any kind.
 * See the LICENSE file in the root of the repository for full license terms.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#pragma once

#ifdef DEBUG

// Must: define SPDLOG_ACTIVE_LEVEL before `#include "spdlog/spdlog.h"`
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#define SPDLOG_TRACE_ON
#define SPDLOG_DEBUG_ON

#if !defined(__PRETTY_FUNCTION__) && !defined(__GNUC__)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/formatter.h>
#include <mutex>

namespace nx
{

  class SLog
  {
  public:

    static inline std::shared_ptr< spdlog::logger > log;

    static void initializeNullWriter()
    {
      std::scoped_lock lock( sm_mutex );
      if ( !log )
      {
        log = spdlog::null_logger_mt( "nx_null" );
      }
    }

    static void initializeConsole()
    {
      std::scoped_lock lock( sm_mutex );
      if ( !log )
      {
        log = spdlog::stdout_color_mt( "nx_stdout" );
        log->set_pattern( "[%L][%t][%H:%M:%S.%e][%!:%#] %v" );
        log->set_level( spdlog::level::trace );
      }
    }

    static void initializeFileWriter( const std::string & file )
    {
      std::scoped_lock lock( sm_mutex );
      if ( !log )
      {
        log = spdlog::basic_logger_mt( "nx_writer", file );
        log->set_pattern( "[%L][%t][%H:%M:%S.%e][%!:%#] %v" );
        log->set_level( spdlog::level::trace );
      }
    }

  private:

    static inline std::mutex sm_mutex {};

  };

}

#ifndef PLOG_LOGGER
#define PLOG_LOGGER

#define LOG_DEBUG( ... ) SPDLOG_LOGGER_DEBUG( nx::SLog::log, __VA_ARGS__ )
#define LOG_INFO( ... ) SPDLOG_LOGGER_INFO( nx::SLog::log, __VA_ARGS__ )
#define LOG_ERROR( ... ) SPDLOG_LOGGER_ERROR( nx::SLog::log, __VA_ARGS__ )
#define LOG_WARN( ... ) SPDLOG_LOGGER_WARN( nx::SLog::log, __VA_ARGS__ )
#define LOG_CRITICAL( ... ) SPDLOG_LOGGER_CRITICAL( nx::SLog::log, __VA_ARGS__ )

#endif

#else

#ifndef PLOG_LOGGER
#define PLOG_LOGGER

#define LOG_DEBUG( ... )
#define LOG_INFO( ... )
#define LOG_ERROR( ... )
#define LOG_WARN( ... )
#define LOG_CRITICAL( ... )

#endif

#endif