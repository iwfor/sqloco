/*
 * This file was automaticly generated by clo++ version $Id$
 * http://pmade.org/pjones/software/clo++/
 */

/** @file
 * This is the main header file (clo.h).
 * You can control the name of this header file using the
 * cxx_header_file variable.
**/

// you can control the include guard
// name with the cxx_header_def variable
#ifndef __clo_h__
#define __clo_h__

// standard includes
#include <stdexcept>
#include <string>
#include <vector>

/// the namespace can be controlled using the cxx_namespace variable
namespace clo {
    /**
     * a list of autothrow ids for the autoexcept exception class
     * so that you know which option caused the exception.
    **/
    enum autothrow { 
	autothrow_help
    };

    /**
     * the following struct is used to hold the values of
     * the options. It has a constructor that sets all the option 
     * values to their default value.
    **/
    struct options {
	options (void) { }

	std::string host;
	std::string password;
	std::string port;
	std::string username;
    }; // end options struct
 
    /**
     * the following struct is used to record the location
     * on the command line where the option was found.
    **/
    struct option_locations {
	typedef int size_type;
	size_type host;
	size_type password;
	size_type port;
	size_type username;
    }; // end option location struct

    /**
     * if there are any errors while parsing the command
     * line, the following class will be thrown.
    **/
    class exception : public std::runtime_error {
    public:
	exception (const std::string &what_arg)
	    : runtime_error(what_arg) { }
    };

    /**
     * if any option that is a autothrow option is present
     * on the command line, this exception class will be
     * thrown when that option is parsed.
    **/
    class autoexcept : public exception {
    public:
	// constructor
	autoexcept (autothrow id, const std::string &message)
	    : exception(message), autothrow_(id) { }
    
	/**
	 * get the autothrow enum member for the autothrow
	 * option that caused the exception.
	**/
	autothrow get_autothrow_id (void) const
	{ return autothrow_; }
    private:
	autothrow autothrow_;
    };

    /**
     * the parser class is the main command
     * line parsing class. You can control the name of it
     * using the cxx_class variable.
    **/
    class parser {
    public:
	/// default constructor
	parser (void);
    
	/// parse the command line
	void parse(int argc, char *argv[], bool call_finalize=true);

	/// finalize all parsing (eg. check for mandatory options)
	void finalize (void);

	/// get a list of nonoptions from the command line
	const std::vector<std::string>& get_non_options (void) const
	{ return non_options_; }

	/// get the main options
	const options& get_options (void) const
	{ return options_; }

	/// get the main option locations
	const option_locations& get_locations (void) const
	{ return locations_; }
    private:
	options options_;
	option_locations locations_;

	enum options_enum {
	    option_username,
		option_password,
		option_host,
		option_port
	} openum_;

	enum parser_state { state_option, state_value, state_consume } state_;
	std::vector<std::string> non_options_;

	enum opsource { source_cl, source_cf };
	void parse_element      (const char *element, int position, opsource source);
	void parse_short_option (char option, int position, opsource source);
	void parse_long_option  (const char *option, int position, opsource source);
	void parse_value        (const char *value);
    }; // end clo::parser class
} // end clo namespace
#endif
