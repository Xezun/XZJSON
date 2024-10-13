//
//  Example001Models.h
//  Example
//
//  Created by 徐臻 on 2024/10/12.
//

#import <Foundation/Foundation.h>
@import XZJSON;

NS_ASSUME_NONNULL_BEGIN

@interface Example001Human : NSObject <XZJSONCoding>
@property (nonatomic, copy) NSString *identifier;
@property (nonatomic, copy) NSString *name;
@property (nonatomic) NSInteger age;
@end

@class Example001Student;
@interface Example001Teacher : Example001Human <XZJSONCoding, XZJSONDecoding>
@property (nonatomic, copy) NSArray<Example001Student *> *students;
@property (nonatomic, copy) NSString *school;
@end

@interface Example001Student : Example001Human <XZJSONEncoding>
@property (nonatomic, weak) Example001Teacher *teacher;
@end

NS_ASSUME_NONNULL_END
