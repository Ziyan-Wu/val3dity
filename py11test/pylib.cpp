#include <val3dity.h>
#include "nlohmann-json/json.hpp"
using json = nlohmann::json;
#include <fstream>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/complex.h>
#include <pybind11/functional.h>
#include <pybind11/chrono.h>

namespace py = pybind11;

int add(int i, int j) {
    //    printf("C++ being called!! %d %d\n",i,j);
    return i + j;
}

int add_arg(int i, int j) {
    //    printf("C++ being called!! %d %d\n",i,j);
    return i + j;
}

int add_default(int i, int j) {
    //    printf("C++ being called!! %d %d\n",i,j);
    return i + j;
}

int sub(int i, int j) {
    return i - j;
}


int sumsum(int k){
    int temp = add_default(1,2);
    int res = add(temp,k);
    return res;
}

bool judge(int num1, int num2){
    if (num1 > num2){
        return true;
    }
    else{ return false;}
}

double dou_add(double a, double b){
    return a+b;
}

///////////////////----class----////////////////////////////
class Hello{
    public:
        Hello(){}
        void say(const std::string s){
            std::cout << s << std::endl;
        }
};



///////////////////----struct----////////////////////////////
struct Pet{
    Pet(const std::string &name) : name(name){}
    void setName(const std::string &name_){name = name_;}
    const std::string &getName() const {return name;}

    std::string name;
};

struct Point {
    float x, y, z;

    Point() {
        x = 0.0;
        y = 0.0;
        z = 0.0;
    }

    Point(const float &x, const float &y, const float &z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    float &operator[](const int &coordinate) {
        if (coordinate == 0) return x;
        else if (coordinate == 1) return y;
        else if (coordinate == 2) return z;
        else
            assert(false);
    }

    float operator[](const int &coordinate) const {
        if (coordinate == 0) return x;
        else if (coordinate == 1) return y;
        else if (coordinate == 2) return z;
        else
            assert(false);
    }

    const Point operator+(const Point &other) const {
        return Point(x + other.x, y + other.y, z + other.z);
    }

    const Point operator-(const Point &other) const {
        return Point(x - other.x, y - other.y, z - other.z);
    }

    const Point operator*(const float &other) const {
        return Point(x * other, y * other, z * other);
    }

    const Point operator/(const float &other) const {
        return Point(x / other, y / other, z / other);
    }
};

double addpoint(){
    Point a (1,2,3);
    Point b (0,1,1);
    Point c = a+b;
    double end = c[0] + c[1] + c[2];
    return end;
}


///////////////////----val3dity----////////////////////////////
bool vc(json& j,
        double tol_snap=0.001,
        double planarity_d2p_tol=0.01,
        double planarity_n_tol=20.0,
        double overlap_tol=-1.0){
    return val3dity::validate_cityjson(j,tol_snap,planarity_d2p_tol,planarity_n_tol,overlap_tol);
}



///////////////////----pybind11----////////////////////////////
// first: module name
PYBIND11_MODULE(kf_cpp, m) {
    m.doc() = "C++ implementation wrappers"; // optional module docstring

    // add functions
    m.def("add", &add, "A function which adds two numbers");
    m.def("add_arg", &add_arg, "A function which adds two numbers with args",py::arg("i"), py::arg("j"));
//    m.def("add_default", &add_default, "A function which adds two numbers with default",py::arg("i")=8, py::arg("j")=7);
    m.def("new",&dou_add,"test add double type");

    m.def("sub", &sub, "A function which sub two numbers");

    // attributes
    m.attr("age") = 18;
    py::object gender = py::cast("male");
    m.attr("gender") = gender;
    m.attr("name") = "Sam";

    //bool tesy
    m.def("judge",&judge,"if first number greater the second return True else False");

    //
    // sumsum call add_default, but add_default not be pybind11 --okay
    m.def("sumsum",&sumsum,"3+k=");
    // addpoint use struct Point, but Point not be pybind11 --okay
    m.def("addpoint",&addpoint,"should be 1+3+4=");


    //val3dity
    m.def("vc", &vc, "A function validate cityjson",
            py::arg("j"),  py::arg("tol_snap")=0.001,py::arg("planarity_d2p_tol")=0.01,
            py::arg("planarity_n_tol")=20.0,py::arg("overlap_tol")=-1.0
            );

    // -----------class---------------
    py::class_<Hello>(m,"Hello")
            .def(py::init())
            .def("say",&Hello::say);

    // ----------struct--------------
    py::class_<Pet>(m,"Pet")
            .def(py::init<const std::string &> ())
            .def("setName", &Pet::setName)
            .def("getName", &Pet::getName);


}
