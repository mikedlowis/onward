#!/usr/bin/env ruby
require './modules/build-system/setup'

#------------------------------------------------------------------------------
# Environment Definitions
#------------------------------------------------------------------------------
# Define the default compiler environment
base_env = BuildEnv.new do |env|
  env.build_root = 'build/'
  # Compiler options
  env["CFLAGS"] += ['-DLEAK_DETECT_LEVEL=1', '--std=c99', '-Wall', '-Wextra']#, '-Werror']
  env["CPPPATH"] += Dir['source/**/']
end

# Define the release environment
main_env = base_env.clone do |env|
  env.build_root = 'build/release/'
  env["CFLAGS"] += ['-O3']
end

# Define the test environment
test_env = base_env.clone do |env|
  env.build_root = 'build/test/'
  env["CPPPATH"] += Dir['modules/atf/source/**/']
  env['CFLAGS'] +=  ['-O0']
  if Opts[:profile].include? "coverage"
    env['CFLAGS']  << '--coverage'
    env['LDFLAGS'] << '--coverage'
  end
end

#------------------------------------------------------------------------------
# Test Build Targets
#------------------------------------------------------------------------------
unless Opts[:profile].include? "no-tests"
  test_env.Program('onward-tests', [
      'source/onward/onward.c',
      'modules/atf/source/atf.c'] +
      Dir['tests/**/*.c'])
  test_env.Command('Unit Tests', ['./onward-tests'], 'CMD' => ['./onward-tests'])
end

#------------------------------------------------------------------------------
# Release Build Targets
#------------------------------------------------------------------------------
main_env.Library('libonward.a', FileList['source/onward/*.c'])
main_env.Program('onward', ['source/main.c', 'libonward.a'])

