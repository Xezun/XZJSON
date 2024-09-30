//
//  XZJSONPropertyDescriptor.h
//  XZJSON
//
//  Created by 徐臻 on 2024/9/29.
//

#import <Foundation/Foundation.h>
#import <XZJSON/XZObjcDescriptor.h>
#import <XZJSON/XZJSONDescriptor.h>

NS_ASSUME_NONNULL_BEGIN

/// A property info in object model.
@interface XZJSONPropertyDescriptor : NSObject {
    @package
    NSString *_name;             ///< property's name
    XZObjcType _type;        ///< property's type
    XZJSONEncodingNSType _nsType;    ///< property's Foundation type
    BOOL _isCNumber;             ///< is c number type
    Class _cls;                  ///< property's class, or nil
    Class _elementClass;           ///< container's generic class, or nil if threr's no generic class
    SEL _getter;                 ///< getter, or nil if the instances cannot respond
    SEL _setter;                 ///< setter, or nil if the instances cannot respond
    BOOL _isKVCCompatible;       ///< YES if it can access with key-value coding
    BOOL _isStructAvailableForKeyedArchiver; ///< YES if the struct can encoded with keyed archiver/unarchiver
    BOOL _hasCustomClassFromDictionary; ///< class/generic class implements +modelCustomClassForDictionary:
    
    /*
     property->key:       _mappedToKey:key     _mappedToKeyPath:nil            _mappedToKeyArray:nil
     property->keyPath:   _mappedToKey:keyPath _mappedToKeyPath:keyPath(array) _mappedToKeyArray:nil
     property->keys:      _mappedToKey:keys[0] _mappedToKeyPath:nil/keyPath    _mappedToKeyArray:keys(array)
     */
    NSString *_mappedToKey;      ///< the key mapped to
    NSArray<NSString *> *_mappedToKeyPath;   ///< the key path mapped to (nil if the name is not key path)
    NSArray *_mappedToKeyArray;  ///< the key(NSString) or keyPath(NSArray) array (nil if not mapped to multiple keys)
    XZObjcPropertyDescriptor *_objcDescriptor;  ///< property's info
    XZJSONPropertyDescriptor *_next; ///< next meta if there are multiple properties mapped to the same key.
}

+ (XZJSONPropertyDescriptor *)descriptorWithClass:(XZObjcClassDescriptor *)aClass property:(XZObjcPropertyDescriptor *)aProperty elementClass:(nullable Class)elementClass;

@end



NS_ASSUME_NONNULL_END
