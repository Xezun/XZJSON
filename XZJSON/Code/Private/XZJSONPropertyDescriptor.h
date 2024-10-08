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
    /// 描述属性的对象。 property's info
    XZObjcPropertyDescriptor *_property;
    /// 属性对应的。 next meta if there are multiple properties mapped to the same key.
    XZJSONPropertyDescriptor *_next;
    
    /// 属性名。property's name
    NSString *_name;
    /// 属性值类型。property's type
    XZObjcType _type;
    /// 如果是，属性值原生类型。property's Foundation type
    XZJSONEncodingNSType _nsType;
    /// 属性是否为 c 数值。is c number type
    BOOL _isCNumber;
    /// 属性值为对象时，对象的类。 property's class, or nil
    Class _Nullable _class;
    /// 属性为集合对象时，元素的类。 container's generic class, or nil if threr's no generic class
    Class _Nullable _elementClass;
    /// 属性的取值方法。 getter, or nil if the instances cannot respond
    SEL _Nullable _getter;
    /// 属性的设值方法。 setter, or nil if the instances cannot respond
    SEL _Nullable _setter;
    /// 是否支持 kvc 键值编码。 YES if it can access with key-value coding
    BOOL _isKVCCompatible;
    /// 属性是否为可存档的结构体类型。 YES if the struct can encoded with keyed archiver/unarchiver
    BOOL _isNSCodingStruct;
    
    /*
     property->key:       _mappedToKey:key     _mappedToKeyPath:nil            _mappedToKeyArray:nil
     property->keyPath:   _mappedToKey:keyPath _mappedToKeyPath:keyPath(array) _mappedToKeyArray:nil
     property->keys:      _mappedToKey:keys[0] _mappedToKeyPath:nil/keyPath    _mappedToKeyArray:keys(array)
     */
    /// 映射到属性的 JSON 键名。 the key mapped to
    NSString            * _Nullable _JSONKey;
    /// 映射到属性的 JOSN 键路径。 the key path mapped to (nil if the name is not key path)
    NSArray<NSString *> * _Nullable _JSONKeyPath;
    /// 映射到属性的 JOSN 键集合。 the key(NSString) or keyPath(NSArray) array (nil if not mapped to multiple keys)
    NSArray             * _Nullable _JSONKeyArray;
}

+ (XZJSONPropertyDescriptor *)descriptorWithClass:(XZObjcClassDescriptor *)aClass property:(XZObjcPropertyDescriptor *)property elementClass:(nullable Class)elementClass;

@end



NS_ASSUME_NONNULL_END
