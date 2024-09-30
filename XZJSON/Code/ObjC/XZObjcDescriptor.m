//
//  XZObjcDescriptor.m
//  YYModel <https://github.com/ibireme/YYModel>
//
//  Created by ibireme on 15/5/9.
//  Copyright (c) 2015 ibireme.
//
//  This source code is licensed under the MIT-style license found in the
//  LICENSE file in the root directory of this source tree.
//

#import "XZObjcDescriptor.h"
#import <objc/runtime.h>

XZObjcType XZObjcTypeFromEncoding(const char *typeEncoding) {
    char *type = (char *)typeEncoding;
    if (!type) return XZObjcTypeUnknown;
    size_t len = strlen(type);
    if (len == 0) return XZObjcTypeUnknown;
    
    XZObjcType qualifier = 0;
    bool prefix = true;
    while (prefix) {
        switch (*type) {
            case 'r': {
                qualifier |= XZObjcTypeQualifierConst;
                type++;
            } break;
            case 'n': {
                qualifier |= XZObjcTypeQualifierIn;
                type++;
            } break;
            case 'N': {
                qualifier |= XZObjcTypeQualifierInout;
                type++;
            } break;
            case 'o': {
                qualifier |= XZObjcTypeQualifierOut;
                type++;
            } break;
            case 'O': {
                qualifier |= XZObjcTypeQualifierBycopy;
                type++;
            } break;
            case 'R': {
                qualifier |= XZObjcTypeQualifierByref;
                type++;
            } break;
            case 'V': {
                qualifier |= XZObjcTypeQualifierOneway;
                type++;
            } break;
            default: { prefix = false; } break;
        }
    }

    len = strlen(type);
    if (len == 0) return XZObjcTypeUnknown | qualifier;

    switch (*type) {
        case 'v': return XZObjcTypeVoid | qualifier;
        case 'B': return XZObjcTypeBool | qualifier;
        case 'c': return XZObjcTypeInt8 | qualifier;
        case 'C': return XZObjcTypeUInt8 | qualifier;
        case 's': return XZObjcTypeInt16 | qualifier;
        case 'S': return XZObjcTypeUInt16 | qualifier;
        case 'i': return XZObjcTypeInt32 | qualifier;
        case 'I': return XZObjcTypeUInt32 | qualifier;
        case 'l': return XZObjcTypeInt32 | qualifier;
        case 'L': return XZObjcTypeUInt32 | qualifier;
        case 'q': return XZObjcTypeInt64 | qualifier;
        case 'Q': return XZObjcTypeUInt64 | qualifier;
        case 'f': return XZObjcTypeFloat | qualifier;
        case 'd': return XZObjcTypeDouble | qualifier;
        case 'D': return XZObjcTypeLongDouble | qualifier;
        case '#': return XZObjcTypeClass | qualifier;
        case ':': return XZObjcTypeSEL | qualifier;
        case '*': return XZObjcTypeCString | qualifier;
        case '^': return XZObjcTypePointer | qualifier;
        case '[': return XZObjcTypeCArray | qualifier;
        case '(': return XZObjcTypeUnion | qualifier;
        case '{': return XZObjcTypeStruct | qualifier;
        case '@': {
            if (len == 2 && *(type + 1) == '?')
                return XZObjcTypeBlock | qualifier;
            else
                return XZObjcTypeObject | qualifier;
        }
        default: return XZObjcTypeUnknown | qualifier;
    }
}

@implementation XZObjcIvarDescriptor

- (instancetype)initWithIvar:(Ivar)ivar {
    if (ivar == nil) {
        return nil;
    }
    
    const char * const name = ivar_getName(ivar);
    if (name == nil) {
        return nil;
    }
    
    const char * const typeEncoding = ivar_getTypeEncoding(ivar);
    if (typeEncoding == nil) {
        return nil;
    }
    
    self = [super init];
    if (self) {
        _identity     = ivar;
        _name         = [NSString stringWithUTF8String:name];
        _offset       = ivar_getOffset(ivar);
        _typeEncoding = [NSString stringWithUTF8String:typeEncoding];
        _type         = XZObjcTypeFromEncoding(typeEncoding);
    }
    return self;
}

@end

@implementation XZObjcMethodDescriptor

- (instancetype)initWithMethod:(Method)method {
    if (method == nil) {
        return nil;
    }
    
    const char * const name = sel_getName(_sel);
    if (name == nil) {
        return nil;
    }
    
    const char * const typeEncoding = method_getTypeEncoding(method);
    if (typeEncoding == nil) {
        return nil;
    }
    
    self = [super init];
    if (self) {
        _identity   = method;
        _sel        = method_getName(method);
        _imp        = method_getImplementation(method);
        _name       = [NSString stringWithUTF8String:name];
        _typeEncoding = [NSString stringWithUTF8String:typeEncoding];
        
        char *returnType = method_copyReturnType(method);
        if (returnType == nil) {
            _returnTypeEncoding = [NSString stringWithUTF8String:returnType];
            free(returnType);
        }
        
        unsigned int const count = method_getNumberOfArguments(method);
        if (count > 0) {
            NSMutableArray *argumentTypes = [NSMutableArray arrayWithCapacity:count];
            for (unsigned int i = 0; i < count; i++) {
                char *argumentType = method_copyArgumentType(method, i);
                NSString *type = argumentType ? [NSString stringWithUTF8String:argumentType] : nil;
                [argumentTypes addObject:type ? type : @""];
                if (argumentType) free(argumentType);
            }
            _argumentTypeEncodings = argumentTypes;
        }
    }
    return self;
}

@end

@implementation XZObjcPropertyDescriptor

- (instancetype)initWithProperty:(objc_property_t)property {
    if (!property) return nil;
    
    const char * const name = property_getName(property);
    if (name == nil) {
        return nil;
    }
    
    self = [super init];
    if (self == nil) {
        return nil;
    }
    
    _identity = property;
    _name     = [NSString stringWithUTF8String:name];
    
    XZObjcType type = 0;
    unsigned int attrCount;
    objc_property_attribute_t *attrs = property_copyAttributeList(property, &attrCount);
    for (unsigned int i = 0; i < attrCount; i++) {
        switch (attrs[i].name[0]) {
            case 'T': { // Type encoding
                if (attrs[i].value) {
                    _typeEncoding = [NSString stringWithUTF8String:attrs[i].value];
                    type = XZObjcTypeFromEncoding(attrs[i].value);
                    
                    if ((type & XZObjcTypeMask) == XZObjcTypeObject && _typeEncoding.length) {
                        NSScanner *scanner = [NSScanner scannerWithString:_typeEncoding];
                        if (![scanner scanString:@"@\"" intoString:NULL]) continue;
                        
                        NSString *clsName = nil;
                        if ([scanner scanUpToCharactersFromSet: [NSCharacterSet characterSetWithCharactersInString:@"\"<"] intoString:&clsName]) {
                            if (clsName.length) _subtype = objc_getClass(clsName.UTF8String);
                        }
                        
                        NSMutableArray *protocols = nil;
                        while ([scanner scanString:@"<" intoString:NULL]) {
                            NSString* protocol = nil;
                            if ([scanner scanUpToString:@">" intoString: &protocol]) {
                                if (protocol.length) {
                                    if (!protocols) protocols = [NSMutableArray new];
                                    [protocols addObject:protocol];
                                }
                            }
                            [scanner scanString:@">" intoString:NULL];
                        }
                        _protocols = protocols;
                    }
                }
            } break;
            case 'V': { // Instance variable
                if (attrs[i].value) {
                    _ivarName = [NSString stringWithUTF8String:attrs[i].value];
                }
            } break;
            case 'R': {
                type |= XZObjcTypePropertyReadonly;
            } break;
            case 'C': {
                type |= XZObjcTypePropertyCopy;
            } break;
            case '&': {
                type |= XZObjcTypePropertyRetain;
            } break;
            case 'N': {
                type |= XZObjcTypePropertyNonatomic;
            } break;
            case 'D': {
                type |= XZObjcTypePropertyDynamic;
            } break;
            case 'W': {
                type |= XZObjcTypePropertyWeak;
            } break;
            case 'G': {
                type |= XZObjcTypePropertyCustomGetter;
                if (attrs[i].value) {
                    _getter = NSSelectorFromString([NSString stringWithUTF8String:attrs[i].value]);
                }
            } break;
            case 'S': {
                type |= XZObjcTypePropertyCustomSetter;
                if (attrs[i].value) {
                    _setter = NSSelectorFromString([NSString stringWithUTF8String:attrs[i].value]);
                }
            } // break; commented for code coverage in next line
            default: break;
        }
    }
    if (attrs) {
        free(attrs);
        attrs = NULL;
    }
    
    _type = type;
    if (_name.length) {
        if (!_getter) {
            _getter = NSSelectorFromString(_name);
        }
        if (!_setter) {
            _setter = NSSelectorFromString([NSString stringWithFormat:@"set%@%@:", [_name substringToIndex:1].uppercaseString, [_name substringFromIndex:1]]);
        }
    }
    return self;
}

@end

@implementation XZObjcClassDescriptor {
    BOOL _isValid;
}

- (instancetype)initWithClass:(Class)cls {
    NSParameterAssert(cls != nil);
    self = [super init];
    if (self) {
        _isValid = NO;
        _identity = cls;
        _identitySuper = class_getSuperclass(cls);
        _isMeta = class_isMetaClass(cls);
        if (!_isMeta) {
            _identityMeta = objc_getMetaClass(class_getName(cls));
        }
        _name = NSStringFromClass(cls);
        [self activateIfNeeded];

        _superDescriptor = [self.class descriptorForClass:_identitySuper];
    }
    return self;
}

- (void)activateIfNeeded {
    if (_isValid) {
        return;
    }
    _isValid = YES;
    _ivars = nil;
    _methods = nil;
    _properties = nil;
    
    Class cls = self.identity;
    unsigned int methodCount = 0;
    Method *methods = class_copyMethodList(cls, &methodCount);
    if (methods) {
        NSMutableDictionary *methodInfos = [NSMutableDictionary new];
        _methods = methodInfos;
        for (unsigned int i = 0; i < methodCount; i++) {
            XZObjcMethodDescriptor *info = [[XZObjcMethodDescriptor alloc] initWithMethod:methods[i]];
            if (info.name) methodInfos[info.name] = info;
        }
        free(methods);
    }
    unsigned int propertyCount = 0;
    objc_property_t *properties = class_copyPropertyList(cls, &propertyCount);
    if (properties) {
        NSMutableDictionary *propertyInfos = [NSMutableDictionary new];
        _properties = propertyInfos;
        for (unsigned int i = 0; i < propertyCount; i++) {
            XZObjcPropertyDescriptor *info = [[XZObjcPropertyDescriptor alloc] initWithProperty:properties[i]];
            if (info.name) propertyInfos[info.name] = info;
        }
        free(properties);
    }
    
    unsigned int ivarCount = 0;
    Ivar *ivars = class_copyIvarList(cls, &ivarCount);
    if (ivars) {
        NSMutableDictionary *ivarInfos = [NSMutableDictionary new];
        _ivars = ivarInfos;
        for (unsigned int i = 0; i < ivarCount; i++) {
            XZObjcIvarDescriptor *info = [[XZObjcIvarDescriptor alloc] initWithIvar:ivars[i]];
            if (info.name) ivarInfos[info.name] = info;
        }
        free(ivars);
    }
    
    if (!_ivars) _ivars = @{};
    if (!_methods) _methods = @{};
    if (!_properties) _properties = @{};
    
    _isValid = NO;
}

- (void)invalidate {
    _isValid = NO;
}

- (BOOL)isValid {
    return _isValid;
}

+ (instancetype)descriptorForClass:(Class)cls {
    if (!cls) return nil;
    static CFMutableDictionaryRef classCache;
    static CFMutableDictionaryRef metaCache;
    static dispatch_once_t onceToken;
    static dispatch_semaphore_t lock;
    dispatch_once(&onceToken, ^{
        classCache = CFDictionaryCreateMutable(CFAllocatorGetDefault(), 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        metaCache = CFDictionaryCreateMutable(CFAllocatorGetDefault(), 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        lock = dispatch_semaphore_create(1);
    });
    dispatch_semaphore_wait(lock, DISPATCH_TIME_FOREVER);
    XZObjcClassDescriptor *meta = CFDictionaryGetValue(class_isMetaClass(cls) ? metaCache : classCache, (__bridge const void *)(cls));
    if (!meta) {
        meta = [[XZObjcClassDescriptor alloc] initWithClass:cls];
        CFDictionarySetValue(meta.isMeta ? metaCache : classCache, (__bridge const void *)(cls), (__bridge const void *)(meta));
    } else if (!meta->_isValid) {
        [meta activateIfNeeded];
    }
    dispatch_semaphore_signal(lock);
    return meta;
}

+ (instancetype)descriptorNamed:(NSString *)className {
    Class cls = NSClassFromString(className);
    return [self descriptorForClass:cls];
}

@end
