<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [Jinja2CppLight](#jinja2cpplight)
- [How to use?](#how-to-use)
  - [overview](#overview)
  - [examples](#examples)
- [Building](#building)
  - [Building on linux](#building-on-linux)
    - [Pre-requisites](#pre-requisites)
    - [Method](#method)
  - [Building on Windows](#building-on-windows)
    - [Pre-requisites:](#pre-requisites)
    - [Procedure](#procedure)
- [Running unittests](#running-unittests)
- [License](#license)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# Jinja2CppLight
(very) lightweight version of Jinja2 for C++

Lightweight templating engine for C++, based on Jinja2
* no dependencies, everything you need to build is included
* templates follow Jinja2 syntax
* supports:
  * variable substitution
  * for loops
  * including nested for loops
  * if statements - partially: only if variable exists or not

# How to use?

## overview

* variable substitution: `{{somevar}}` will be replaced by the value of `somevar`
* for loops: `{% for somevar in range(5) %}...{% endfor %}` will be expanded, assigning somevar the values of 
0, 1, 2, 3 and 4, accessible as normal template variables, ie in this case `{{somevar}}`

## examples

Simple example of using variable substitution:
```c++
    Template mytemplate( R"d(
        This is my {{avalue}} template.  It's {{secondvalue}}...
        Today's weather is {{weather}}.
    )d" );
    mytemplate.setValue( "avalue", 3 );
    mytemplate.setValue( "secondvalue", 12.123f );
    mytemplate.setValue( "weather", "rain" );
    string result = mytemplate.render();
    cout << result << endl;
    string expectedResult = R"d(
        This is my 3 template.  It's 12.123...
        Today's weather is rain.
    )d";
    EXPECT_EQ( expectedResult, result );
```

eg, example of using loops, eg to unroll some loops, maybe in an OpenCL kernel:
```
    Template mytemplate( R"d(
{% for i in range(its) %}a[{{i}}] = image[{{i}}];
{% for j in range(2) %}b[{{j}}] = image[{{j}}];
{% endfor %}{% endfor %}
)d" );
    mytemplate.setValue( "its", 3 );
    string result = mytemplate.render();
    string expectedResult = R"d(
a[0] = image[0];
b[0] = image[0];
b[1] = image[1];
a[1] = image[1];
b[0] = image[0];
b[1] = image[1];
a[2] = image[2];
b[0] = image[0];
b[1] = image[1];

)d";
    EXPECT_EQ( expectedResult, result );
```
```
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
````

simple if condition:
```
    const std::string source = "abc{% if its %}def{% endif %}ghi";
    Template mytemplate(source);
    mytemplate.setValue("its", 3);
    const std::string result = mytemplate.render();
    std::cout << "[" << result << "]" << endl;
    const std::string expectedResult = "abcdefghi";
    EXPECT_EQ(expectedResult, result);
```

# Building

## Building on linux

### Pre-requisites

* cmake
* g++
* make

### Method

```bash
git clone git@github.com:hughperkins/Jinja2CppLight.git
cd Jinja2CppLight
mkdir build
cd build
cmake ..
make
```

## Building on Windows

### Pre-requisites:

* Visual Studio 2013 Community, or similar
* cmake
* git (eg msys-git)

### Procedure

* use git to clone git@github.com:hughperkins/Jinja2CppLight.git
* open cmake, and use it to generate visual studio project files, from the checked out repository
* open visual studio, and build the generate visual studio project files, as `Release`

# Running unittests

After building, as above, on linux:
```bash
./jinja2cpplight_unittests
```
on Windows, from the Release directory folder:
```
jinja2cpplight_unittests
```

# Related projects

For an alternative approach, using lua as a templating scripting language, see [luacpptemplater](https://github.com/hughperkins/luacpptemplater)

# License

Mozilla Public License

