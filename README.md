# Jinja2CppLight
(very) lightweight version of Jinja2 for C++

Lightweight templating engine for C++, based on Jinja2
* no dependencies, everything you need to build is included
* templates follow Jinja2 syntax
* supports:
  * variable substitution
  * for loops
  * including nested for loops

# Why didn't I use one of the existing template libraries?

* one of them has boost as a dependency, which is a major task to build/make available on Windows
* one of them has QT as a dependency
* one of them supports only variable substitution, no for loops :-P
* one of them uses xml syntax

After looking around a bit, I wrote this one :-)

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
./unittests
```
on Windows, from the Release directory folder:
```
unittests
```

# License

Mozilla Public License

