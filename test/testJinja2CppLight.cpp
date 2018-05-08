// Copyright Hugh Perkins 2015 hughperkins at gmail
//
// This Source Code Form is subject to the terms of the Mozilla Public License,
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.

#include <iostream>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "test/gtest_supp.h"

#include "Jinja2CppLight.h"

using namespace std;
using namespace Jinja2CppLight;

TEST( testJinja2CppLight, basicsubstitution ) {
    string source = R"DELIM(
        This is my {{avalue}} template.  It's {{secondvalue}}...
        Today's weather is {{weather}}. Values list: {{valuesList}}
    )DELIM";

    Template mytemplate( source );
    mytemplate.setValue( "avalue", 3 );
    mytemplate.setValue( "secondvalue", 12.123f );
    mytemplate.setValue( "weather", "rain" );
    mytemplate.setValue( "valuesList", TupleValue::create(10, 20.256, "Hello World!"));
    string result = mytemplate.render();
    cout << result << endl;
    string expectedResult = R"DELIM(
        This is my 3 template.  It's 12.123...
        Today's weather is rain. Values list: {10, 20.256, Hello World!}
    )DELIM";
    EXPECT_EQ( expectedResult, result );
}
/*
TEST( testJinja2CppLight, vectorsubstitution ) {
    string source = R"DELIM(
        Here's the list: {{ values }}...
        {% if maybe %}{{ maybe }}{% endif %}
        {% if maybenot %}{{ maybenot }}{% endif %}
        We're going to print it anyway: {{ maybenot }}
    )DELIM";
    
    std::vector<std::string> values = { "one", "two", "three" };
    std::vector<std::string> maybe = { "There is one" };
    std::vector<std::string> maybenot;
    Template mytemplate( source );
    mytemplate.setValue( "values", values );
    mytemplate.setValue( "maybe", maybe );
    mytemplate.setValue( "maybenot", maybenot );
    string result = mytemplate.render();
    const string expectedResult = R"DELIM(
        Here's the list: one two three...
        There is one
        
        We're going to print it anyway: 
    )DELIM";
    EXPECT_EQ( expectedResult, result );
}
TEST( testJinja2CppLight, forloop ) {
    string source = R"DELIM(
        Shopping list:{% for item in items %}
          - {{ item }}{% endfor %}
    )DELIM";
    
    std::vector<std::string> shopList = { "eggs", "milk", "vodka" };
    Template mytemplate0( source );
    mytemplate0.setValue( "items", shopList );
    string result = mytemplate0.render();
    string expectedResult = R"DELIM(
        Shopping list:
          - eggs
          - milk
          - vodka
    )DELIM";
    EXPECT_EQ( expectedResult, result );
    
    shopList.clear();
    Template mytemplate1( source );
    mytemplate1.setValue( "items", shopList );
    result = mytemplate1.render();
    expectedResult = R"DELIM(
        Shopping list:
    )DELIM";
    EXPECT_EQ( expectedResult, result );
}
*/
TEST( testSpeedTemplates, namemissing ) {
    string source = R"DELIM(
        This is my {{avalue}} template.
    )DELIM";

    Template mytemplate( source );
    bool threw = false;
    try {
        string result = mytemplate.render();
    } catch( render_error &e ) {
        EXPECT_EQ( string("name avalue not defined"), e.what() );
        threw = true;
    }
    EXPECT_EQ( true, threw );
}
TEST( testSpeedTemplates, loop ) {
    string source = R"DELIM(
{% for i in range(its) %}
            a[{{i}}] = image[{{i}}];
{% endfor %}
    )DELIM";

    Template mytemplate( source );
    mytemplate.setValue( "its", 3 );
    string result = mytemplate.render();
    cout << result << endl;
    string expectedResult = R"DELIM(

            a[0] = image[0];

            a[1] = image[1];

            a[2] = image[2];

    )DELIM";
    EXPECT_EQ( expectedResult, result );
}

TEST( testSpeedTemplates, tupleloop ) {
    string source = R"DELIM(
{% for i in its %}
            a[{{i}}] = image[{{i}}];
{% endfor %}
    )DELIM";

    Template mytemplate( source );
    mytemplate.setValue( "its", TupleValue::create(0, 1.1, "2abc") );
    string result = mytemplate.render();
    cout << result << endl;
    string expectedResult = R"DELIM(

            a[0] = image[0];

            a[1.1] = image[1.1];

            a[2abc] = image[2abc];

    )DELIM";
    EXPECT_EQ( expectedResult, result );
}

TEST( testSpeedTemplates, nestedloop ) {
    string source = R"DELIM(
{% for i in range(its) %}a[{{i}}] = image[{{i}}];
{% for j in range(2) %}b[{{j}}] = image[{{j}}];
{% endfor %}{% endfor %}
)DELIM";

    Template mytemplate( source );
    mytemplate.setValue( "its", 3 );
    string result = mytemplate.render();
    cout << "[" << result << "]" << endl;
    string expectedResult = R"DELIM(
a[0] = image[0];
b[0] = image[0];
b[1] = image[1];
a[1] = image[1];
b[0] = image[0];
b[1] = image[1];
a[2] = image[2];
b[0] = image[0];
b[1] = image[1];

)DELIM";
    EXPECT_EQ( expectedResult, result );
}

TEST(testSpeedTemplates, ifTrueTest) {
    const std::string source = "abc{% if True %}def{% endif %}ghi";
    Template mytemplate( source );
    const string result = mytemplate.render();
    std::cout << "[" << result << "]" << endl;
    const std::string expectedResult = "abcdefghi";

    EXPECT_EQ(expectedResult, result);
}

TEST(testSpeedTemplates, ifFalseTest) {
    const std::string source = "abc{% if False %}def{% endif %}ghi";
    Template mytemplate(source);
    const string result = mytemplate.render();
    std::cout << "[" << result << "]" << endl;
    const std::string expectedResult = "abcghi";

    EXPECT_EQ(expectedResult, result);
}

TEST(testSpeedTemplates, ifNotTrueTest) {
    const std::string source = "abc{% if not True %}def{% endif %}ghi";
    Template mytemplate(source);
    const string result = mytemplate.render();
    std::cout << "[" << result << "]" << endl;
    const std::string expectedResult = "abcghi";

    EXPECT_EQ(expectedResult, result);
}

TEST(testSpeedTemplates, ifNotFalseTest) {
    const std::string source = "abc{% if not False %}def{% endif %}ghi";
    Template mytemplate(source);
    const string result = mytemplate.render();
    std::cout << "[" << result << "]" << endl;
    const std::string expectedResult = "abcdefghi";

    EXPECT_EQ(expectedResult, result);
}

TEST(testSpeedTemplates, ifVariableExitsTest) {
    const std::string source = "abc{% if its %}def{% endif %}ghi";

    {
        Template mytemplate(source);
        const std::string expectedResultNoVariable = "abcghi";
        const std::string result = mytemplate.render();
        EXPECT_EQ(expectedResultNoVariable, result);
    }

    {
        Template mytemplate(source);
        mytemplate.setValue("its", 3);
        const std::string result = mytemplate.render();
        std::cout << "[" << result << "]" << endl;
        const std::string expectedResult = "abcdefghi";
        EXPECT_EQ(expectedResult, result);
    }
}

TEST(testSpeedTemplates, ifVariableDoesntExitTest) {
    const std::string source = "abc{% if not its %}def{% endif %}ghi";

    {
        Template mytemplate(source);
        const std::string expectedResultNoVariable = "abcdefghi";
        const std::string result = mytemplate.render();
        EXPECT_EQ(expectedResultNoVariable, result);
    }

    {
        Template mytemplate(source);
        mytemplate.setValue("its", 3);
        const std::string result = mytemplate.render();
        std::cout << "[" << result << "]" << endl;
        const std::string expectedResult = "abcghi";
        EXPECT_EQ(expectedResult, result);
    }
}

TEST(testSpeedTemplates, ifUnexpectedExpression) {
    const std::string source = "abc{% if its is defined %}def{% endif %}ghi";
    Template myTemplate(source);
    bool threw = false;
    try {
        myTemplate.render();
    }
    catch (const render_error &e) {
        EXPECT_EQ(std::string("Unexpected expression after variable name: is"), e.what());
        threw = true;
    }
    EXPECT_EQ(true, threw);
}
