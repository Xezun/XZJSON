//
//  XZObjcDescriptor.h
//  YYModel <https://github.com/ibireme/YYModel>
//
//  Created by ibireme on 15/5/9.
//  Copyright (c) 2015 ibireme.
//
//  This source code is licensed under the MIT-style license found in the
//  LICENSE file in the root directory of this source tree.
//

#import <Foundation/Foundation.h>
#import <objc/runtime.h>

NS_ASSUME_NONNULL_BEGIN

/// 目前已知的 objc 所有数据类型的枚举。
/// Type encoding's type.
typedef NS_OPTIONS(NSUInteger, XZObjcType) {
    XZObjcTypeMask       = 0xFF,    ///< mask of type value
    XZObjcTypeUnknown    = 0,       ///< unknown
    XZObjcTypeVoid       = 1,       ///< void
    XZObjcTypeBool       = 2,       ///< bool
    XZObjcTypeInt8       = 3,       ///< char / BOOL
    XZObjcTypeUInt8      = 4,       ///< unsigned char
    XZObjcTypeInt16      = 5,       ///< short
    XZObjcTypeUInt16     = 6,       ///< unsigned short
    XZObjcTypeInt32      = 7,       ///< int
    XZObjcTypeUInt32     = 8,       ///< unsigned int
    XZObjcTypeInt64      = 9,       ///< long long
    XZObjcTypeUInt64     = 10,      ///< unsigned long long
    XZObjcTypeFloat      = 11,      ///< float
    XZObjcTypeDouble     = 12,      ///< double
    XZObjcTypeLongDouble = 13,      ///< long double
    XZObjcTypeObject     = 14,      ///< id
    XZObjcTypeClass      = 15,      ///< Class
    XZObjcTypeSEL        = 16,      ///< SEL
    XZObjcTypeBlock      = 17,      ///< block
    XZObjcTypePointer    = 18,      ///< void*
    XZObjcTypeStruct     = 19,      ///< struct
    XZObjcTypeUnion      = 20,      ///< union
    XZObjcTypeCString    = 21,      ///< char*
    XZObjcTypeCArray     = 22,      ///< char[10] (for example)
    
    XZObjcTypeQualifierMask   = 0xFF00,     ///< mask of qualifier
    XZObjcTypeQualifierConst  = 1 << 8,     ///< const
    XZObjcTypeQualifierIn     = 1 << 9,     ///< in
    XZObjcTypeQualifierInout  = 1 << 10,    ///< inout
    XZObjcTypeQualifierOut    = 1 << 11,    ///< out
    XZObjcTypeQualifierBycopy = 1 << 12,    ///< bycopy
    XZObjcTypeQualifierByref  = 1 << 13,    ///< byref
    XZObjcTypeQualifierOneway = 1 << 14,    ///< oneway
    
    XZObjcTypePropertyMask         = 0xFF0000,  ///< mask of property
    XZObjcTypePropertyReadonly     = 1 << 16,   ///< readonly
    XZObjcTypePropertyCopy         = 1 << 17,   ///< copy
    XZObjcTypePropertyRetain       = 1 << 18,   ///< retain
    XZObjcTypePropertyNonatomic    = 1 << 19,   ///< nonatomic
    XZObjcTypePropertyWeak         = 1 << 20,   ///< weak
    XZObjcTypePropertyCustomGetter = 1 << 21,   ///< getter=
    XZObjcTypePropertyCustomSetter = 1 << 22,   ///< setter=
    XZObjcTypePropertyDynamic      = 1 << 23,   ///< @dynamic
};


/// 将 objc 类型编码转换为类型枚举。
///
/// Get the type from a Type-Encoding string.
///
/// 1. [Objective-C Runtime Programming Guide - Type Encodings](https://developer.apple.com/library/mac/documentation/Cocoa/Conceptual/ObjCRuntimeGuide/Articles/ocrtTypeEncodings.html)
///
/// 2. [Objective-C Runtime Programming Guide - Declared Properties](https://developer.apple.com/library/mac/documentation/Cocoa/Conceptual/ObjCRuntimeGuide/Articles/ocrtPropertyIntrospection.html)
///
/// - Parameter typeEncoding: A Type-Encoding string.
/// - Returns: The encoding type.
FOUNDATION_EXPORT XZObjcType XZObjcTypeFromEncoding(const char *typeEncoding);


/// 描述实例变量的对象。
///
/// Instance variable information.
@interface XZObjcIvarDescriptor : NSObject
@property (nonatomic, assign, readonly) Ivar raw;          ///< ivar opaque struct
@property (nonatomic, strong, readonly) NSString *name;         ///< Ivar's name
@property (nonatomic, assign, readonly) ptrdiff_t offset;       ///< Ivar's offset
@property (nonatomic, strong, readonly) NSString *typeEncoding; ///< Ivar's type encoding
@property (nonatomic, assign, readonly) XZObjcType type;        ///< Ivar's type
- (nullable instancetype)initWithIvar:(Ivar)ivar NS_DESIGNATED_INITIALIZER;
- (instancetype)init NS_UNAVAILABLE;
@end


/// 描述方法的对象。
///
/// Method information.
@interface XZObjcMethodDescriptor : NSObject
@property (nonatomic, assign, readonly) Method raw;                ///< method opaque struct
@property (nonatomic, strong, readonly) NSString *name;                 ///< method name
@property (nonatomic, assign, readonly) SEL sel;                        ///< method's selector
@property (nonatomic, assign, readonly) IMP imp;                        ///< method's implementation
@property (nonatomic, strong, readonly) NSString *typeEncoding;         ///< method's parameter and return types
@property (nonatomic, strong, readonly, nullable) NSString *returnTypeEncoding;   ///< return value's type
@property (nonatomic, strong, readonly, nullable) NSArray<NSString *> *argumentTypeEncodings; ///< array of arguments' type
- (nullable instancetype)initWithMethod:(Method)method NS_DESIGNATED_INITIALIZER;
- (instancetype)init NS_UNAVAILABLE;
@end

/// 描述属性的对象。
///
/// Property information.
@interface XZObjcPropertyDescriptor : NSObject
@property (nonatomic, assign, readonly) objc_property_t raw;                   ///< property's opaque struct
@property (nonatomic, copy, readonly) NSString *name;                             ///< property's name
@property (nonatomic, assign, readonly) XZObjcType type;                            ///< property's type
@property (nonatomic, assign, readonly, nullable) Class subtype;                    ///< may be nil
@property (nonatomic, strong, readonly) NSString *typeEncoding;                     ///< property's encoding value
@property (nonatomic, strong, readonly, nullable) NSString *ivarName;               ///< property's ivar name
@property (nonatomic, strong, readonly, nullable) NSArray<NSString *> *protocols;   ///< may nil
@property (nonatomic, assign, readonly) SEL getter;                                 ///< getter (nonnull)
@property (nonatomic, assign, readonly) SEL setter;                                 ///< setter (nonnull)
- (nullable instancetype)initWithProperty:(objc_property_t)property NS_DESIGNATED_INITIALIZER;
- (instancetype)init NS_UNAVAILABLE;
@end

/// 描述类的对象。
///
/// Class information for a class.
@interface XZObjcClassDescriptor : NSObject

/// 当前类，当前对象所描述的类。
@property (nonatomic, assign, readonly) Class raw;
/// 描述当前类的超类的对象。
@property (nonatomic, readonly, nullable) XZObjcClassDescriptor *superDescriptor;
/// 描述当前类的元类的对象。
@property (nonatomic, readonly, nullable) XZObjcClassDescriptor *metaDescriptor;


/// whether this class is meta class
@property (nonatomic, readonly) BOOL isMeta;
/// class name
@property (nonatomic, copy, readonly) NSString *name;
/// ivars
@property (nonatomic, strong, readonly, nullable) NSDictionary<NSString *, XZObjcIvarDescriptor *>     *ivars;
/// methods
@property (nonatomic, strong, readonly, nullable) NSDictionary<NSString *, XZObjcMethodDescriptor *>   *methods;
/// properties
@property (nonatomic, strong, readonly, nullable) NSDictionary<NSString *, XZObjcPropertyDescriptor *> *properties;

- (instancetype)init NS_UNAVAILABLE;

/**
 If this method returns `YES`, you should stop using this instance and call
 `classInfoWithClass` or `classInfoWithClassName` to get the updated class info.
 
 @return Whether this class info need update.
 */
@property (nonatomic, readonly) BOOL isValid;

/**
 If the class is changed (for example: you add a method to this class with
 'class_addMethod()'), you should call this method to refresh the class info cache.
 
 After called this method, `needUpdate` will returns `YES`, and you should call 
 'classInfoWithClass' or 'classInfoWithClassName' to get the updated class info.
 */
- (void)invalidate;

/**
 Get the class info of a specified Class.
 
 @discussion This method will cache the class info and super-class info
 at the first access to the Class. This method is thread-safe.
 
 @param cls A class.
 @return A class info, or nil if an error occurs.
 */
+ (nullable XZObjcClassDescriptor *)descriptorForClass:(Class)cls;

/**
 Get the class info of a specified Class.
 
 @discussion This method will cache the class info and super-class info
 at the first access to the Class. This method is thread-safe.
 
 @param className A class name.
 @return A class info, or nil if an error occurs.
 */
+ (nullable XZObjcClassDescriptor *)descriptorNamed:(NSString *)className;

@end

NS_ASSUME_NONNULL_END
