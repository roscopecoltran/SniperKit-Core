#import <Foundation/Foundation.h>

#include "ApplicationPath.h"

std::string iOSGetApplicationPath()
{
    NSBundle* bundle = [NSBundle mainBundle];
    NSString* path = [bundle resourcePath];
    return std::string([path UTF8String]);
}
