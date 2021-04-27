/*=============================================================================
    Boost.Wave: A Standard compliant C++ preprocessor library

    http://www.boost.org/

    Copyright (c) 2001-2012 Hartmut Kaiser. Distributed under the Boost
    Software License, Version 1.0. (See accompanying file
    LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
//  Include Wave itself
#include "boost/wave.hpp"

///////////////////////////////////////////////////////////////////////////////
// Include the lexer stuff
#include "boost/wave/cpplexer/cpp_lex_token.hpp"    // token class
#include "boost/wave/cpplexer/cpp_lex_iterator.hpp" // lexer class
#include "trace_macro_expansion.hpp"
//  This token type is one of the central types used throughout the library.
//  It is a template parameter to some of the public classes and instances
//  of this type are returned from the iterators.
	typedef boost::wave::cpplexer::lex_token<> token_type;

//  The template boost::wave::cpplexer::lex_iterator<> is the lexer type to
//  to use as the token source for the preprocessing engine. It is
//  parametrized with the token type.
	typedef boost::wave::cpplexer::lex_iterator<token_type> lex_iterator_type;

//  This is the resulting context type. The first template parameter should
//  match the iterator type used during construction of the context
//  instance (see below). It is the type of the underlying input stream.
	typedef boost::wave::context<std::string::iterator, lex_iterator_type>
		context_type;

//namespace po = boost::program_options;
namespace fs = boost::filesystem;

//std::vector<std::string> sysPath=
//{
//"/home/nick/Downloads/gcc-10.2.0/gcc-10.2.0-debug-libcpp-install/lib/gcc/x86_64-pc-linux-gnu/10.2.0/../../../../include/c++/10.2.0",
//"/home/nick/Downloads/gcc-10.2.0/gcc-10.2.0-debug-libcpp-install/lib/gcc/x86_64-pc-linux-gnu/10.2.0/../../../../include/c++/10.2.0/x86_64-pc-linux-gnu",
//"/home/nick/Downloads/gcc-10.2.0/gcc-10.2.0-debug-libcpp-install/lib/gcc/x86_64-pc-linux-gnu/10.2.0/../../../../include/c++/10.2.0/backward",
//"/home/nick/Downloads/gcc-10.2.0/gcc-10.2.0-debug-libcpp-install/lib/gcc/x86_64-pc-linux-gnu/10.2.0/include",
//"/usr/local/include",
//"/home/nick/Downloads/gcc-10.2.0/gcc-10.2.0-debug-libcpp-install/include",
//"/home/nick/Downloads/gcc-10.2.0/gcc-10.2.0-debug-libcpp-install/lib/gcc/x86_64-pc-linux-gnu/10.2.0/include-fixed",
//"/usr/include/x86_64-linux-gnu",
//"/usr/include",
//};
std::vector<std::string> userPath=
{
"/home/nick/Downloads/boost-1_72-install/include",
"/home/nick/Downloads/boost_1_72_0/libs/wave/tool/"
};

void readSystemIncludePathFile(std::vector<std::string>& v)
{
	std::ifstream in("include.txt");
	std::string str;
	while (std::getline(in,  str, '\n'))
	{
		v.push_back(str);
	}
}

void readPredefinedMacroFile(std::vector<std::string>& v)
{
	std::ifstream in("macro.txt");
	std::string str;
	while (std::getline(in,  str, '\n'))
	{
		std::string subStr=str.substr(std::string("#define ").size());
		size_t i=subStr.find(' ');
		if (i!=std::string::npos)
		{
			subStr[i]='=';
		}
		v.push_back(subStr);
	}
}
bool list_include_names(context_type const& ctx, std::string filename)
{
	std::ofstream listnames_out;
	fs::path listnames_file (boost::wave::util::create_path(filename));

	if (listnames_file != "-") {
		listnames_file = boost::wave::util::complete_path(listnames_file);
		boost::wave::util::create_directories(
			boost::wave::util::branch_path(listnames_file));
		listnames_out.open(listnames_file.string().c_str());
		if (!listnames_out.is_open()) {
			std::cerr << "wave: could not open file for macro name listing: "
				 << listnames_file.string() << std::endl;
			return false;
		}
	}
	else {
		listnames_out.copyfmt(std::cout);
		listnames_out.clear(std::cout.rdstate());
		static_cast<std::basic_ios<char> &>(listnames_out).rdbuf(std::cout.rdbuf());
	}

}
bool list_macro_names(context_type const& ctx, std::string filename)
{
// open file for macro names listing
	std::ofstream macronames_out;
	fs::path macronames_file (boost::wave::util::create_path(filename));

	if (macronames_file != "-") {
		macronames_file = boost::wave::util::complete_path(macronames_file);
		boost::wave::util::create_directories(
			boost::wave::util::branch_path(macronames_file));
		macronames_out.open(macronames_file.string().c_str());
		if (!macronames_out.is_open()) {
			std::cerr << "wave: could not open file for macro name listing: "
				 << macronames_file.string() << std::endl;
			return false;
		}
	}
	else {
		macronames_out.copyfmt(std::cout);
		macronames_out.clear(std::cout.rdstate());
		static_cast<std::basic_ios<char> &>(macronames_out).rdbuf(std::cout.rdbuf());
	}

// simply list all defined macros and its definitions
	typedef context_type::const_name_iterator name_iterator;
	name_iterator end = ctx.macro_names_end();
	for (name_iterator it = ctx.macro_names_begin(); it != end; ++it)
	{
		typedef std::vector<context_type::token_type> parameters_type;

		bool has_pars = false;
		bool predef = false;
		context_type::position_type pos;
		parameters_type pars;
		context_type::token_sequence_type def;

		if (ctx.get_macro_definition(*it, has_pars, predef, pos, pars, def))
		{
			macronames_out<<pos<<std::endl;
			macronames_out << (predef ? "-P" : "-D") << *it;
			if (has_pars) {
			// list the parameter names for function style macros
				macronames_out << "(";
				parameters_type::const_iterator pend = pars.end();
				for (parameters_type::const_iterator pit = pars.begin();
					 pit != pend; /**/)
				{
					macronames_out << (*pit).get_value();
					if (++pit != pend)
						macronames_out << ", ";
				}
				macronames_out << ")";
			}
			macronames_out << "=";

		// print the macro definition
			context_type::token_sequence_type::const_iterator dend = def.end();
			for (context_type::token_sequence_type::const_iterator dit = def.begin();
				 dit != dend; ++dit)
			{
				macronames_out << (*dit).get_value();
			}

			macronames_out << std::endl;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// main entry point
int main(int argc, char *argv[])
{
	// current file position is saved for exception handling
	boost::wave::util::file_position_type current_position;


    	//[quick_start_main
    //  The following preprocesses the input file given by argv[1].
    //  Open and read in the specified input file.
	std::ifstream instream("/home/nick/eclipse-2021/waveTest/src/waveTest.cpp");
	std::string instring;

	if (!instream.is_open()) {
		std::cerr << "Could not open input file: " << argv[1] << std::endl;
		return -2;
	}
	instream.unsetf(std::ios::skipws);
	instring = std::string(std::istreambuf_iterator<char>(instream.rdbuf()),
							std::istreambuf_iterator<char>());


    //  The preprocessor iterator shouldn't be constructed directly. It is
    //  generated through a wave::context<> object. This wave:context<> object
    //  is additionally used to initialize and define different parameters of
    //  the actual preprocessing (not done here).
    //
    //  The preprocessing of the input stream is done on the fly behind the
    //  scenes during iteration over the range of context_type::iterator_type
    //  instances.
	std::string default_outfile;
	std::ofstream out("/tmp/output.txt");
	bool allowed=true;
	bool notAllowed=false;
	trace_flags enable_trace = (trace_flags)(trace_macros|trace_includes|trace_guards);
	trace_macro_expansion<token_type> hooks(notAllowed,
			notAllowed, out, out, out,
			out, enable_trace, notAllowed, allowed,
			default_outfile);
	hooks.enable_macro_counting();
	context_type ctx (instring.begin(), instring.end(), "/home/nick/eclipse-2021/waveTest/src/waveTest.cpp");
	try
	{
        ctx.set_language(
                        boost::wave::language_support(
                            boost::wave::support_cpp0x
                         |  boost::wave::support_option_convert_trigraphs
                         |  boost::wave::support_option_long_long
                         |  boost::wave::support_option_emit_line_directives
        #if BOOST_WAVE_SUPPORT_PRAGMA_ONCE != 0
                         |  boost::wave::support_option_include_guard_detection
        #endif
        #if BOOST_WAVE_EMIT_PRAGMA_DIRECTIVES != 0
                         |  boost::wave::support_option_emit_pragma_directives
        #endif
                         |  boost::wave::support_option_insert_whitespace
                        ));
	// At this point you may want to set the parameters of the
	// preprocessing as include paths and/or predefined macros.
        std::vector<std::string> sysPath;
        readSystemIncludePathFile(sysPath);
        for (auto s : sysPath)
        {
        	ctx.add_sysinclude_path(s.c_str());
        }
        for (auto s: userPath)
        {
        	ctx.add_include_path(s.c_str());
        }
        std::vector<std::string> v;
        readPredefinedMacroFile(v);
        for (auto s:v)
        {
        	ctx.add_macro_definition(s, true);
        }
//        ctx.remove_macro_definition(std::string("__cplusplus"), true);
//        ctx.add_macro_definition(std::string("__cplusplus=201703L"), true);
        //ctx.
        //#define __cplusplus 201703L
		//ctx.add_macro_definition(...);
    //  Get the preprocessor iterators and use them to generate the token
    //  sequence.
        context_type::iterator_type first = ctx.begin();
        context_type::iterator_type last = ctx.end();

    //  The input stream is preprocessed for you while iterating over the range
    //  [first, last). The dereferenced iterator returns tokens holding
    //  information about the preprocessed input stream, such as token type,
    //  token value, and position.
        while (first != last) {
            current_position = (*first).get_position();
            std::cout << (*first).get_value();
            ++first;
        }
//]
    }
    catch (boost::wave::cpp_exception const& e) {
    // some preprocessing error
        std::cerr
            << e.file_name() << "(" << e.line_no() << "): "
            << e.description() << std::endl;
        list_macro_names(ctx, "/tmp/macro.output");
        list_include_names(ctx, "/tmp/include.output");
        return 2;
    }
    catch (std::exception const& e) {
    // use last recognized token to retrieve the error position
        std::cerr
            << current_position.get_file()
            << "(" << current_position.get_line() << "): "
            << "exception caught: " << e.what()
            << std::endl;
        return 3;
    }
    catch (...) {
    // use last recognized token to retrieve the error position
        std::cerr
            << current_position.get_file()
            << "(" << current_position.get_line() << "): "
            << "unexpected exception caught." << std::endl;
        return 4;
    }
    return 0;
}
