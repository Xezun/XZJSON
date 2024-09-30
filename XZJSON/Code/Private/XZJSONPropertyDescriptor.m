//
//  XZJSONPropertyDescriptor.m
//  XZJSON
//
//  Created by 徐臻 on 2024/9/29.
//

#import "XZJSONPropertyDescriptor.h"
#import "XZJSONDefines.h"

@implementation XZJSONPropertyDescriptor

+ (XZJSONPropertyDescriptor *)descriptorWithClass:(XZObjcClassDescriptor *)aClass property:(XZObjcPropertyDescriptor *)aProperty elementClass:(Class)elementClass {
    
    // support pseudo generic class with protocol name
    if (!elementClass && aProperty.protocols) {
        for (NSString *protocol in aProperty.protocols) {
            Class cls = objc_getClass(protocol.UTF8String);
            if (cls) {
                elementClass = cls;
                break;
            }
        }
    }
    
    XZJSONPropertyDescriptor *descriptor = [self new];
    descriptor->_name = aProperty.name;
    descriptor->_type = aProperty.type;
    descriptor->_objcDescriptor = aProperty;
    descriptor->_elementClass   = elementClass;
    
    if ((descriptor->_type & XZObjcTypeMask) == XZObjcTypeObject) {
        descriptor->_nsType = XZJSONEncodingNSTypeFromClass(aProperty.cls);
    } else {
        descriptor->_isCNumber = XZObjcTypeIsCNumber(descriptor->_type);
    }
    
    if ((descriptor->_type & XZObjcTypeMask) == XZObjcTypeStruct) {
        /// It seems that NSKeyedUnarchiver cannot decode NSValue except these structs:
        static NSSet *types = nil;
        static dispatch_once_t onceToken;
        dispatch_once(&onceToken, ^{
            NSMutableSet *set = [NSMutableSet new];
            // 32 bit
            [set addObject:@"{CGSize=ff}"];
            [set addObject:@"{CGPoint=ff}"];
            [set addObject:@"{CGRect={CGPoint=ff}{CGSize=ff}}"];
            [set addObject:@"{CGAffineTransform=ffffff}"];
            [set addObject:@"{UIEdgeInsets=ffff}"];
            [set addObject:@"{UIOffset=ff}"];
            // 64 bit
            [set addObject:@"{CGSize=dd}"];
            [set addObject:@"{CGPoint=dd}"];
            [set addObject:@"{CGRect={CGPoint=dd}{CGSize=dd}}"];
            [set addObject:@"{CGAffineTransform=dddddd}"];
            [set addObject:@"{UIEdgeInsets=dddd}"];
            [set addObject:@"{UIOffset=dd}"];
            types = set;
        });
        if ([types containsObject:aProperty.typeEncoding]) {
            descriptor->_isStructAvailableForKeyedArchiver = YES;
        }
    }
    descriptor->_class = aProperty.cls;
    
    if (elementClass) {
        descriptor->_hasCustomClassFromDictionary = [elementClass respondsToSelector:@selector(forwardingClassForJSONDictionary:)];
    } else if (descriptor->_class && descriptor->_nsType == XZJSONEncodingUnknown) {
        descriptor->_hasCustomClassFromDictionary = [descriptor->_class respondsToSelector:@selector(forwardingClassForJSONDictionary:)];
    }
    
    if (aProperty.getter) {
        if ([aClass.identity instancesRespondToSelector:aProperty.getter]) {
            descriptor->_getter = aProperty.getter;
        }
    }
    
    if (aProperty.setter) {
        if ([aClass.identity instancesRespondToSelector:aProperty.setter]) {
            descriptor->_setter = aProperty.setter;
        }
    }
    
    if (descriptor->_getter && descriptor->_setter) {
        /*
         KVC invalid type:
         long double
         pointer (such as SEL/CoreFoundation object)
         */
        switch (descriptor->_type & XZObjcTypeMask) {
            case XZObjcTypeBool:
            case XZObjcTypeInt8:
            case XZObjcTypeUInt8:
            case XZObjcTypeInt16:
            case XZObjcTypeUInt16:
            case XZObjcTypeInt32:
            case XZObjcTypeUInt32:
            case XZObjcTypeInt64:
            case XZObjcTypeUInt64:
            case XZObjcTypeFloat:
            case XZObjcTypeDouble:
            case XZObjcTypeObject:
            case XZObjcTypeClass:
            case XZObjcTypeBlock:
            case XZObjcTypeStruct:
            case XZObjcTypeUnion: {
                descriptor->_isKVCCompatible = YES;
            } break;
            default: break;
        }
    }
    
    return descriptor;
}
@end
