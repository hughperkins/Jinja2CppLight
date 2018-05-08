// Copyright Hugh Perkins 2015 hughperkins at gmail
//
// This Source Code Form is subject to the terms of the Mozilla Public License,
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.

// the intent here is to create a templates library that:
// - is based on Jinja2 syntax
// - doesn't depend on boost, qt, etc ...

// for now, will handle:
// - variable substitution, ie {{myvar}}
// - for loops, ie {% for i in range(myvar) %}

#pragma once

#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <memory>
#include "stringhelper.h"

#define VIRTUAL virtual
#define STATIC static

namespace Jinja2CppLight {

class render_error : public std::runtime_error {
public:
    render_error( const std::string &what ) :
        std::runtime_error( what ) {
    }
};

class Value {
public:
    virtual ~Value() {}
    virtual std::string render() = 0;
    virtual bool isTrue() const = 0;
};
class IntValue : public Value {
public:
    int value;
    IntValue( int value ) :
        value( value ) {
    }
    virtual std::string render() {
        return toString( value );
    }
    bool isTrue() const {
        return value != 0;
    }
};
class FloatValue : public Value {
public:
    float value;
    FloatValue( float value ) :
        value( value ) {
    }
    virtual std::string render() {
        return toString( value );
    }
    bool isTrue() const {
        return value != 0.0;
    }
};
class StringValue : public Value {
public:
    std::string value;
    StringValue( std::string value ) :
        value( value ) {
    }
    virtual std::string render() {
        return value;
    }
    bool isTrue() const {
        return !value.empty();
    }
};

class TupleValue : public Value
{
public:
    std::vector<std::shared_ptr<Value>> values;

    TupleValue &addValue( int value ) {
        values.push_back(std::make_shared<IntValue>(value));
        return *this;
    }
    TupleValue &addValue( double value ) {
        values.push_back(std::make_shared<FloatValue>(value));
        return *this;
    }
    TupleValue &addValue( std::string value ) {
        values.push_back(std::make_shared<StringValue>(std::move(value)));
        return *this;
    }
    TupleValue &addValue( TupleValue value ) {
        values.push_back(std::make_shared<TupleValue>(std::move(value)));
        return *this;
    }

    bool isTrue() const {
        return !values.empty();
    }

    virtual std::string render() {
        std::string result = "{";
        bool isFirst = true;

        for (auto& val : values) {
            if (isFirst)
                isFirst = false;
            else
                result += ", ";

            result += val ? val->render() : "<empty>";
        }

        result += "}";
        return result;
    }

    template<typename ... Args>
    static TupleValue create(Args&& ... args) {
        TupleValue result;

        createImpl (result, std::forward<Args>(args)...);

        return result;
    }
private:
    static void createImpl(TupleValue&) {
    }

    template<typename Arg, typename ... Args>
    static void createImpl(TupleValue& result, Arg&& arg, Args&& ... args) {
        result.addValue(std::forward<Arg>(arg));
        createImpl (result, std::forward<Args>(args)...);
    }
};

class Root;
class ControlSection;
typedef std::map < std::string, std::shared_ptr<Value> > ValueMap;

class Template {
public:
    std::string sourceCode;

    ValueMap valueByName;

    // [[[cog
    // import cog_addheaders
    // cog_addheaders.add(classname='Template')
    // ]]]
    // generated, using cog:
    Template( std::string sourceCode );
    STATIC bool isNumber( std::string astring, int *p_value );
    VIRTUAL ~Template();
    Template &setValue( std::string name, int value );
    Template &setValue( std::string name, float value );
    Template &setValue( std::string name, std::string value );
    Template&setValue( std::string name, TupleValue value);
    std::string render();
    void print(ControlSection *section);
    int eatSection( int pos, ControlSection *controlSection );
    STATIC std::string doSubstitutions( std::string sourceCode, const ValueMap &valueByName );

    // [[[end]]]
};

class ControlSection {
public:
    
    virtual ~ControlSection() { sections.clear(); }
    
    std::vector< std::unique_ptr<ControlSection> >sections;
    virtual std::string render( ValueMap &valueByName ) = 0;
    virtual void print() {
        print("");
    }
    virtual void print(std::string prefix) = 0;
};

class Container : public ControlSection {
public:
//    std::vector< ControlSection * >sections;
    int sourceCodePosStart;
    int sourceCodePosEnd;

//    std::string render( ValueMap valueByName );
    virtual void print( std::string prefix ) {
        std::cout << prefix << "Container ( " << sourceCodePosStart << ", " << sourceCodePosEnd << " ) {" << std::endl;
        for( int i = 0; i < (int)sections.size(); i++ ) {
            sections[i]->print( prefix + "    " );
        }
        std::cout << prefix << "}" << std::endl;
    }
};

class ForRangeSection : public ControlSection {
public:
    int loopStart;
    int loopEnd;
    std::string varName;
    int startPos;
    int endPos;
    std::string render( ValueMap &valueByName ) {
        std::string result = "";
//        bool nameExistsBefore = false;
        if( valueByName.find( varName ) != valueByName.end() ) {
            throw render_error("variable " + varName + " already exists in this context" );
        }
        valueByName[varName] = std::make_shared<IntValue>( 0 );
        for (auto i = loopStart; i < loopEnd; ++i ){
            dynamic_cast<IntValue*>(valueByName[varName].get())->value = i;
            for( size_t j = 0; j < sections.size(); j++ ) {
                result += sections[j]->render( valueByName );
            }
        }
        valueByName.erase( varName );
        return result;
    }
    //Container *contents;
    virtual void print( std::string prefix ) {
        std::cout << prefix << "For ( " << varName << " in range( ) {" << std::endl;
        for( int i = 0; i < (int)sections.size(); i++ ) {
            sections[i]->print( prefix + "    " );
        }
        std::cout << prefix << "}" << std::endl;
    }
};

class ForSection : public ControlSection {
public:
    std::string varName;
    std::string tupVarName;
    virtual std::string render( ValueMap &valueByName ) {
        std::string result = "";
        if( valueByName.find( varName ) != valueByName.end() ) {
            throw render_error("variable " + varName + " already exists in this context" );
        }
        
        const std::shared_ptr<Value> &val = valueByName.at( tupVarName ); // throws if something happened to the TupleValue
        const TupleValue *tupValue = dynamic_cast< TupleValue * >( val.get() );
        if (!tupValue) {
            throw render_error("variable " + tupVarName + " no longer valid in context" );
        }
        const std::vector<std::shared_ptr<Value>> &tupValues = tupValue->values;
        for ( auto itr = tupValues.cbegin(); itr != tupValues.cend(); ++itr ) {
            valueByName[ varName ] = *itr;
            for( std::size_t j = 0; j < sections.size(); ++j) {
                result += sections[j]->render( valueByName );
            }
            valueByName.erase( varName );
        }
        
        return result;
    }
    virtual void print( std::string prefix ) {
        std::cout << prefix << "For ( " << varName << " in " << tupVarName << " ) {" << std::endl;
        for( std::size_t i = 0; i < sections.size(); i++ ){
            sections[i]->print( prefix + "    " );
        }
        std::cout << prefix << "}" << std::endl;
    }
};

class Code : public ControlSection {
public:
//    vector< ControlSection * >sections;
    int startPos;
    int endPos;
    std::string templateCode;

    std::string render();
    virtual void print( std::string prefix ) {
        std::cout << prefix << "Code ( " << startPos << ", " << endPos << " ) {" << std::endl;
        for( int i = 0; i < (int)sections.size(); i++ ) {
            sections[i]->print( prefix + "    " );
        }
        std::cout << prefix << "}" << std::endl;
    }
    virtual std::string render( ValueMap &valueByName ) {
//        std::string templateString = sourceCode.substr( startPos, endPos - startPos );
//        std::cout << "Code section, rendering [" << templateCode << "]" << std::endl;
        std::string processed = Template::doSubstitutions( templateCode, valueByName );
//        std::cout << "Code section, after rendering: [" << processed << "]" << std::endl;
        return processed;
    }
};

class Root : public ControlSection {
public:
    virtual ~Root() {}
    virtual std::string render( ValueMap &valueByName ) {
        std::string resultString = "";
        for( int i = 0; i < (int)sections.size(); i++ ) {
            resultString += sections[i]->render( valueByName );
        }
        return resultString;
    }
    virtual void print(std::string prefix) {
        std::cout << prefix << "Root {" << std::endl;
        for( int i = 0; i < (int)sections.size(); i++ ) {
            sections[i]->print( prefix + "    " );
        }
        std::cout << prefix << "}" << std::endl;
    }
};

class IfSection : public ControlSection {
public:
    IfSection(const std::string& expression) {
        parseIfCondition(expression);
    }

    std::string render(ValueMap &valueByName) {
        std::stringstream ss;
        const bool expressionValue = computeExpression(valueByName);
        if (expressionValue) {
            for (size_t j = 0; j < sections.size(); j++) {
                ss << sections[j]->render(valueByName);
            }
        }
        const std::string renderResult = ss.str();
        return renderResult;
    }

    void print(std::string prefix) {
        std::cout << prefix << "if ( "
            << ((m_isNegation) ? "not " : "")
            << m_variableName << " ) {" << std::endl;
        if (true) {
            for (int i = 0; i < (int)sections.size(); i++) {
                sections[i]->print(prefix + "    ");
            }
        }
        std::cout << prefix << "}" << std::endl;
    }

private:
    //? It determines m_isNegation and m_variableName from @param[in] expression.
    //? @param[in] expression E.g. "if not myVariable" where myVariable is set by myTemplate.setValue( "myVariable", <any_value> );
    //?                       The result of this statement is false if myVariable is initialized.
    void parseIfCondition(const std::string& expression);

    bool computeExpression(const ValueMap &valueByName) const;

    bool m_isNegation; ///< Tells whether is there "if not" or just "if" at the begin of expression.
    std::string m_variableName; ///< This simple "if" implementation allows single variable condition only.
};

}

