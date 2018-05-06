// Copyright Hugh Perkins 2015 hughperkins at gmail
//
// This Source Code Form is subject to the terms of the Mozilla Public License,
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>

#include "stringhelper.h"

#include "Jinja2CppLight.h"

using namespace std;

namespace
{
    const std::string JINJA2_TRUE = "True";
    const std::string JINJA2_FALSE = "False";
    const std::string JINJA2_NOT = "not";
}

namespace Jinja2CppLight {

#undef VIRTUAL
#define VIRTUAL
#undef STATIC
#define STATIC

Template::Template( std::string sourceCode ) :
    sourceCode( sourceCode ),
    root( new Root() )
{}

STATIC bool Template::isNumber( std::string astring, int *p_value ) {
    istringstream in( astring );
    int value;
    if( in >> value && in.eof() ) {
        *p_value = value;
        return true;
    }
    return false;
}
VIRTUAL Template::~Template() {
    valueByName.clear();
}
Template &Template::setValue( std::string name, int value ) {
    valueByName[ name ] = std::make_shared<IntValue>( value );
    return *this;
}
Template &Template::setValue( std::string name, float value ) {
    valueByName[ name ] = std::make_shared<FloatValue>( value );
    return *this;
}
Template &Template::setValue( std::string name, std::string value ) {
    valueByName[ name ] = std::make_shared<StringValue>( std::move(value) );
    return *this;
}

Template&Template::setValue( std::string name, TupleValue value) {
    valueByName[ name ] = std::make_shared<TupleValue>( std::move(value) );
    return *this;

}
std::string Template::render() {
    size_t finalPos = eatSection(0, root.get() );
    if( finalPos != sourceCode.length() ) {
        throw render_error("some sourcecode found at end: " + sourceCode.substr( finalPos ) );
    }
    return root->render(valueByName);
}

void Template::print(ControlSection *section) {
    section->print("");
}

// pos should point to the first character that has sourcecode inside the control section controlSection
// return value should be first character of the control section end part (ie first char of {% endfor %} type bit)
int Template::eatSection( int pos, ControlSection *controlSection ) {
//    int pos = 0;
//    vector<string> tokenStack;
//    string updatedString = "";
    while( true ) {
//        cout << "pos: " << pos << endl;
        size_t controlChangeBegin = sourceCode.find( "{%", pos );
//        cout << "controlChangeBegin: " << controlChangeBegin << endl;
        if( controlChangeBegin == string::npos ) {
            //updatedString += doSubstitutions( sourceCode.substr( pos ), valueByName );
            std::unique_ptr<Code> code(new Code());
            code->startPos = pos;
            code->endPos = sourceCode.length();
//            code->templateCode = sourceCode.substr( pos, sourceCode.length() - pos );
            code->templateCode = sourceCode.substr( code->startPos, code->endPos - code->startPos );
            controlSection->sections.push_back( std::move(code) );
            return sourceCode.length();
        } else {
            size_t controlChangeEnd = sourceCode.find( "%}", controlChangeBegin );
            if( controlChangeEnd == string::npos ) {
                throw render_error( "control section unterminated: " + sourceCode.substr( controlChangeBegin, 40 ) );
            }
            string controlChange = trim( sourceCode.substr( controlChangeBegin + 2, controlChangeEnd - controlChangeBegin - 2 ) );
            vector<string> splitControlChange = split( controlChange, " " );
            if( splitControlChange[0] == "endfor" || splitControlChange[0] == "endif") {
                if( splitControlChange.size() != 1 ) {
                    throw render_error("control section {% " + controlChange + " unrecognized" );
                }
                std::unique_ptr<Code> code(new Code());
                code->startPos = pos;
                code->endPos = controlChangeBegin;
                code->templateCode = sourceCode.substr( code->startPos, code->endPos - code->startPos );
                controlSection->sections.push_back( std::move(code) );
                return controlChangeBegin;
//                if( tokenStack.size() == 0 ) {
//                    throw render_error("control section {% " + controlChange + " unexpected: no current control stack items" );
//                }
//                if( tokenStack[ tokenStack.size() - 1 ] != "for" ) {
//                    throw render_error("control section {% " + controlChange + " unexpected: current last control stack item is: " + tokenStack[ tokenStack.size() - 1 ] );
//                }
//                cout << "token stack old size: " << tokenStack.size() << endl;
//                tokenStack.erase( tokenStack.end() - 1, tokenStack.end() - 1 );
//                string varToRemove = varNameStack[ (int)tokenStack.size() - 1 ];
//                valueByName.erase( varToRemove );
//                varNameStack.erase( tokenStack.end() - 1, tokenStack.end() - 1 );
//                cout << "token stack new size: " << tokenStack.size() << endl;
            } else if( splitControlChange[0] == "for" ) {
                std::unique_ptr<Code> code(new Code());
                code->startPos = pos;
                code->endPos = controlChangeBegin;
                code->templateCode = sourceCode.substr( code->startPos, code->endPos - code->startPos );
                controlSection->sections.push_back( std::move(code) );

                string varname = splitControlChange[1];
                if( splitControlChange[2] != "in" ) {
                    throw render_error("control section {% " + controlChange + " unexpected: second word should be 'in'" );
                }
                string rangeString = "";
                for( int i = 3; i < (int)splitControlChange.size(); i++ ) {
                    rangeString += splitControlChange[i];
                }
                rangeString = replaceGlobal( rangeString, " ", "" );
                vector<string> splitRangeString = split( rangeString, "(" );
                if( splitRangeString[0] != "range" && splitRangeString.size() > 1 ) {
                    throw render_error("control section {% " + controlChange + " unexpected: third word should start with 'range' or be a variable name" );
                }
                std::shared_ptr<TupleValue> values;
                if ( splitRangeString[0] == "range" ) {
                    if( splitRangeString.size() != 2 ) {
                        throw render_error("control section " + controlChange + " unexpected: should be in format 'range(somevar)' or 'range(somenumber)'" );
                    }
                    string name = split( splitRangeString[1], ")" )[0];
                    //                cout << "for range name: " << name << endl;
                    int endValue = 0;
                    if( isNumber( name, &endValue ) ) {
                    } else {
                        auto p = valueByName.find( name );
                        if( p != valueByName.end() ) {
                            IntValue *intValue = dynamic_cast< IntValue * >( p->second.get() );
                            if ( intValue == 0 ) {
                                throw render_error("for loop range var " + name + " must be an int (but it's not)");
                            }
                            endValue = intValue->value;
                        } else {
                            throw render_error("for loop range var " + name + " not recognized");
                        }
                    }
                    values = std::make_shared<TupleValue>();
                    for ( int idx = 0; idx < endValue; ++ idx )
                        values->addValue(idx);
                } else {
                    auto &name = splitRangeString[0];
                    auto p = valueByName.find( name );
                    if( p != valueByName.end() ) {
                        auto tupleValue = std::dynamic_pointer_cast< TupleValue >( p->second );
                        if ( tupleValue ) {
                            values = tupleValue;
                        } else {
                            throw render_error("for loop range var " + name + " must be an int (but it's not)");
                        }
                    } else {
                        throw render_error("for loop range var " + name + " not recognized");
                    }
                }
//                cout << "for loop start=" << beginValue << " end=" << endValue << endl;
                std::unique_ptr<ForSection> forSection(new ForSection());
                forSection->startPos = controlChangeEnd + 2;
                forSection->values = std::move(values);
                forSection->varName = varname;
                pos = eatSection( controlChangeEnd + 2, forSection.get() );
                size_t controlEndEndPos = sourceCode.find("%}", pos );
                if( controlEndEndPos == string::npos ) {
                    throw render_error("No control end section found at: " + sourceCode.substr(pos ) );
                }
                string controlEnd = sourceCode.substr( pos, controlEndEndPos - pos + 2 );
                string controlEndNorm = replaceGlobal( controlEnd, " ", "" );
                if( controlEndNorm != "{%endfor%}" ) {
                    throw render_error("No control end section found, expected '{% endfor %}', got '" + controlEnd + "'" );
                }
                forSection->endPos = controlEndEndPos + 2;
                controlSection->sections.push_back(std::move(forSection));
                pos = controlEndEndPos + 2;
//                tokenStack.push_back("for");
//                varNameStack.push_back(name);
            } else if (splitControlChange[0] == "if") {
                std::unique_ptr<Code> code(new Code());
                code->startPos = pos;
                code->endPos = controlChangeBegin;
                code->templateCode = sourceCode.substr(code->startPos, code->endPos - code->startPos);
                controlSection->sections.push_back(std::move(code));
                const string word = splitControlChange[1];
                if (JINJA2_TRUE == word)  {
                    ;
                } else if (JINJA2_FALSE == word) {
                    ;
                }
                else if (JINJA2_NOT == word) {
                    ;
                }
                else {
                    ;
                }
                std::unique_ptr<IfSection> ifSection(new IfSection(controlChange));

                pos = eatSection(controlChangeEnd + 2, ifSection.get());
                controlSection->sections.push_back(std::move(ifSection));
                size_t controlEndEndPos = sourceCode.find("%}", pos);
                if (controlEndEndPos == string::npos) {
                    throw render_error("No control end of any section found at: " + sourceCode.substr(pos));
                }
                string controlEnd = sourceCode.substr(pos, controlEndEndPos - pos + 2);
                string controlEndNorm = replaceGlobal(controlEnd, " ", "");
                if (controlEndNorm != "{%endif%}") {
                    throw render_error("No control end section found, expected '{% endif %}', got '" + controlEnd + "'");
                }
                //forSection->endPos = controlEndEndPos + 2;
                pos = controlEndEndPos + 2;

            } else {
                throw render_error("control section {% " + controlChange + " unexpected" );
            }
        }
    }

//    vector<string> controlSplit = split( sourceCode, "{%" );
////    int startI = 1;
////    if( controlSplit.substr(0,2) == "{%" ) {
////        startI = 0;
////    }
//    string updatedString = "";
//    for( int i = 0; i < (int)controlSplit.size(); i++ ) {
//        if( controlSplit[i].substr(0,2) == "{%" ) {
//            vector<string> splitControlPair = split(controlSplit[i], "%}" );
//            string controlString = splitControlPair[0];
//        } else {
//            updatedString += doSubstitutions( controlSplit[i], valueByName );
//        }
//    }
////    string templatedString = doSubstitutions( sourceCode, valueByName );
//    return updatedString;
}
STATIC std::string Template::doSubstitutions( std::string sourceCode, const ValueMap &valueByName ) {
    int startI = 1;
    if( sourceCode.substr(0,2) == "{{" ) {
        startI = 0;
    }
    string templatedString = "";
    vector<string> splitSource = split( sourceCode, "{{" );
    if( startI == 1 ) {
        templatedString = splitSource[0];
    }
    for( size_t i = startI; i < splitSource.size(); i++ ) {
        if (0 == i && splitSource.size() > 1 && splitSource[i].empty()) // Ignoring initial empty section if there are other sections.
        {
            continue;
        }

        vector<string> thisSplit = split( splitSource[i], "}}" );
        string name = trim( thisSplit[0] );
//        cout << "name: " << name << endl;
        auto p = valueByName.find( name );
        if( p == valueByName.end() ) {
            throw render_error( "name " + name + " not defined" );
        }
        auto value = p->second;
        templatedString += value->render();
        if( thisSplit.size() > 0 ) {
            templatedString += thisSplit[1];
        }
    }
    return templatedString;
}

void IfSection::parseIfCondition(const std::string& expression) {
    const std::vector<std::string> splittedExpression = split(expression, " ");
    if (splittedExpression.empty() || splittedExpression[0] != "if") {
        throw render_error("if statement expected.");
    }

    std::size_t expressionIndex = 1;
    if (splittedExpression.size() < expressionIndex + 1) {
        throw render_error("Any expression expected after if statement.");
    }
    m_isNegation = (JINJA2_NOT == splittedExpression[expressionIndex]);
    expressionIndex += (m_isNegation) ? 1 : 0;
    if (splittedExpression.size() < expressionIndex + 1) {
        if (!m_isNegation)
            throw render_error("Any expression expected after if statement.");
        else
            throw render_error("Any expression expected after if not statement.");
    }
    m_variableName = splittedExpression[expressionIndex];
    if (splittedExpression.size() > expressionIndex + 1) {
        throw render_error(std::string("Unexpected expression after variable name: ") + splittedExpression[expressionIndex + 1]);
    }
}

bool IfSection::computeExpression(const ValueMap &valueByName) const {
    if (JINJA2_TRUE == m_variableName) {
        return true ^ m_isNegation;
    }
    else if (JINJA2_FALSE == m_variableName) {
        return false ^ m_isNegation;
    }
    else {
        const bool valueExists = valueByName.count(m_variableName) > 0;
        if (!valueExists) {
            return false ^ m_isNegation;
        }
        return valueByName.at(m_variableName)->isTrue() ^ m_isNegation;
    }
}

}


