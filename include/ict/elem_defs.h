#pragma once
//-- Copyright 2015 Intrig
//-- see https://github.com/intrig/xenon for license

inline element::~element() { 
    if (v) { 
        IT_WARN("deleting " << name()); 
        delete v; 
        v = nullptr; 
        IT_WARN("done");
    }
}


inline element::element(const element& x) {
    IT_WARN("copy");
    line = x.line;
    parser = x.parser;
    tag_ = x.tag_;
    name_ = x.name_;
    v = x.v;
    flags = x.flags;
}

inline element::element(element&& x) {
    IT_WARN("move copy");
    line = x.line;
    parser = x.parser;
    tag_ = x.tag_;
    name_ = x.name_;
    v = x.v;
    flags = x.flags;
    x.v = nullptr;
}

inline element& element::operator=(const element& x) {
    IT_WARN("assignment");
    if (this != &x) {
        if (v) delete v;

        line = x.line;
        parser = x.parser;
        tag_ = x.tag_;
        name_ = x.name_;
        v = x.v;
        flags = x.flags;
    }
    return *this;
}

inline element& element::operator=(element&& x) {
    IT_WARN("move assignment");
    if (this != &x) {
        if (v) delete v;
        line = x.line;
        parser = x.parser;
        tag_ = x.tag_;
        name_ = x.name_;
        v = x.v;
        flags = x.flags;
        x.v = nullptr;
    }
    return *this;
}
