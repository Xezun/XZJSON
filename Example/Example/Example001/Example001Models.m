//
//  Example001Models.m
//  Example
//
//  Created by 徐臻 on 2024/10/12.
//

#import "Example001Models.h"

@implementation Example001Human

+ (NSDictionary<NSString *,id> *)mappingJSONCodingKeys {
    return @{
        @"identifier": @[@"id", @"identifier", @"_id"]
    };
}

@end

@implementation Example001Teacher

+ (NSDictionary<NSString *,id> *)mappingJSONCodingClasses {
    return @{
        @"students": [Example001Student class]
    };
}

+ (NSDictionary<NSString *,id> *)mappingJSONCodingKeys {
    return @{
        @"identifier": @"id",
        @"school": @"school\\.name"
    };
}

- (instancetype)initWithJSONDictionary:(NSDictionary *)JSON {
    // 调用指定初始化方法。
    self = [self init];
    if (self != nil) {
        // 使用 XZJSON 进行初始化。
        [XZJSON object:self decodeWithDictionary:JSON];
        
        // 处理自定义逻辑：关联学生和老师
        [self.students enumerateObjectsUsingBlock:^(Example001Student * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
            obj.teacher = self;
        }];
    }
    return self;
}

@end

@implementation Example001Student

+ (NSArray<NSString *> *)blockedJSONCodingKeys {
    return @[@"teacher"];
}

+ (NSArray<NSString *> *)allowedJSONCodingKeys {
    return nil;
}

@end

