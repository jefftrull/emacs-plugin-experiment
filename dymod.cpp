// A simple C++ plugin for Emacs
#include <iostream>
#include <cassert>

#include <emacs-module.h>

// signal to Emacs that we are libre
void *plugin_is_GPL_compatible = nullptr;

emacs_env * eenv;   // for use by our code

// things we can do with eenv:
// create/free globals
// force function exit
// register and call functions
// get the type of a value and test it for nil, equality
// extract/create ints and floats
// insert strings into buffers
// create/get/set user_ptr (custom C/C++ types embedded for us)
// register "finalizer" (dtor) for user_ptr
// get, set, and determine size of Emacs "vector"

// Very rudimentary binding example, for now.
// You can test all of this with a one-liner:
// emacs -l ./libdymod.so -f dymod-sample-nullary-void-fn --eval='(message "computed %d" (dymod-sample-unary-int-fn 8))'
// it will print "Hello Elisp" twice on the console
// "Foo" and "Bar" also (for the class instances)
// and "computed 16" in the echo area

// Our API
void nullary_void_fn() { std::cout << "Hello Elisp\n"; }
int  unary_int_fn(int i) { return 2*i; }

struct SomeClass {
    SomeClass(std::string d) : data_(std::move(d)) {}
    ~SomeClass() {}
    void DoSomething() const { std::cout << data_ << "\n"; }
private:
    std::string data_;
};

int emacs_module_init(struct emacs_runtime *ert) {
    // get the current environment
    eenv = ert->get_environment(ert);

    // register a some simple functions
    // create the Emacs-side function wrapper object
    emacs_value fn = eenv->make_function(eenv, 0, 0,
                        [](emacs_env *env, ptrdiff_t, emacs_value [], void*) {
                            nullary_void_fn();
                            return env->intern(env, "nil");
                        },
                        "An example nullary void function bound from C++",
                        nullptr);  // a way to supply associated data in the callback

    // we can call it right away if we want to
    emacs_value retval = eenv->funcall(eenv, fn, 0, nullptr);

    // the result should be nil
    assert(!eenv->is_not_nil(eenv, retval));
    (void)retval;   // avoid unused variable error in Release builds

    // we can also bind it to a symbol a la defun:

    // first, create a symbol we want to associate with the function
    emacs_value fname = eenv->intern(eenv, "dymod-sample-nullary-void-fn");

    // next get the fset function from Emacs
    emacs_value fset = eenv->intern(eenv, "fset");

    // call fset
    emacs_value args[]{fname, fn};
    eenv->funcall(eenv, fset, 2, args);

    // now we can call our named function from within Emacs

    // Let's try a more complicated example
    emacs_value fn2 = eenv->make_function(eenv, 1, 1,
                        [](emacs_env *env, ptrdiff_t, emacs_value args[] , void*) {
                            int result = unary_int_fn(env->extract_integer(env, args[0]));
                            return env->make_integer(env, result);
                        },
                        "An example binary int function bound from C++, that doubles its argument",
                        nullptr);

    // call it
    emacs_value args2[]{eenv->make_integer(eenv, 3)};
    retval = eenv->funcall(eenv, fn2, 1, args2);
    assert(eenv->extract_integer(eenv, retval) == 6);  // doubled

    // name it
    emacs_value fname2 = eenv->intern(eenv, "dymod-sample-unary-int-fn");
    emacs_value args3[]{fname2, fn2};
    eenv->funcall(eenv, fset, 2, args3);

    // now a member function
    // This is similar to the other functions but we need to get the "this" pointer
    // back so we know which object it applies to

    auto foo = new SomeClass("Foo");   // yeah this will leak. TODO.
    auto bar = new SomeClass("Bar");

    // a function to execute the DoSomething member of a SomeClass
    auto doer = [](emacs_env *env, ptrdiff_t, emacs_value [] , void* that) {
        static_cast<SomeClass*>(that)->DoSomething();
        return env->intern(env, "nil");
    };

    // now bind it to the two different instances of SomeClass
    emacs_value foo_fn = eenv->make_function(eenv, 0, 0, doer,
                                             "calls foo's DoSomething method",
                                             foo);
    emacs_value bar_fn = eenv->make_function(eenv, 0, 0, doer,
                                             "calls bar's DoSomething method",
                                             bar);

    // and call them
    eenv->funcall(eenv, foo_fn, 0, nullptr);
    eenv->funcall(eenv, bar_fn, 0, nullptr);

    return 0;
}

