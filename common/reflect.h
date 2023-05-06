//
// Created by Administrator on 2023/5/6.
//
#pragma once

/*

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <type_traits>

// 定义可反射类型的基类
class Reflectable {
public:
    virtual ~Reflectable() = default;

    // 获取可反射类型的类型名
    virtual std::string typeName() const = 0;

    // 获取可反射类型的字段列表
    virtual std::vector<std::string> fieldNames() const = 0;

    // 根据字段名获取字段值
    virtual std::string getFieldValue(const std::string& fieldName) const = 0;

    // 根据字段名设置字段值
    virtual void setFieldValue(const std::string& fieldName, const std::string& value) = 0;

    // 输出该类型的所有字段和对应的值
    void dump() const {
        std::cout << "type: " << typeName() << std::endl;
        for(const auto& fieldName : fieldNames()) {
            std::cout << fieldName << ": " << getFieldValue(fieldName) << std::endl;
        }
    }
};

// 反射注册表
class ReflectionRegistry {
public:
    // 注册可反射类型
    template<typename ReflectableType>
    static void registerType() {
        static_assert(std::is_base_of<Reflectable, ReflectableType>::value, "ReflectableType should inherit from Reflectable.");
        // 注册类型及其字段
        typeFieldMap_[ReflectableType::staticTypeName()] = ReflectableType::staticFieldNames();
        // 注册默认构造函数
        defaultConstructors_[ReflectableType::staticTypeName()] = [](){
            return new ReflectableType;
        };
    }

    // 创建类型实例
    static std::unique_ptr<Reflectable> create(const std::string& typeName) {
        auto it = defaultConstructors_.find(typeName);
        if(it == defaultConstructors_.end()) {
            return nullptr;
        }
        return std::unique_ptr<Reflectable>(it->second());
    }

    // 获取类型的字段列表
    static std::vector<std::string> fieldNames(const std::string& typeName) {
        auto it = typeFieldMap_.find(typeName);
        if(it == typeFieldMap_.end()) {
            return std::vector<std::string>();
        }
        return it->second;
    }

private:
    // 类型字段映射表
    static std::unordered_map<std::string, std::vector<std::string>> typeFieldMap_;

    // 类型默认构造函数映射表
    static std::unordered_map<std::string, std::function<Reflectable*()>> defaultConstructors_;
};

std::unordered_map<std::string, std::vector<std::string>> ReflectionRegistry::typeFieldMap_;
std::unordered_map<std::string, std::function<Reflectable*()>> ReflectionRegistry::defaultConstructors_;

// 可反射类型的基本类型定义
#define REFLECTABLE_TYPE(ClassName)                             \
    public:                                                     \
    static std::string staticTypeName() { return #ClassName; }  \
    std::string typeName() const override { return staticTypeName(); } \
    std::vector<std::string> fieldNames() const override { return staticFieldNames(); } \
    static std::vector<std::string> staticFieldNames() {        \
        std::vector<std::string> names;                         \
        reflectionFieldNames(names);                            \
        return names;                                           \
    }                                                           \
    void reflectionFieldNames(std::vector<std::string>& names) const

// 可反射类型的字段定义
#define REFLECTABLE_FIELD(FieldName) \
    names.push_back(#FieldName);

// 可反射类型的字段和getter/setter定义
#define REFLECTABLE_FIELD_GETSET(FieldName, FieldType) \
    names.push_back(#FieldName);                        \
    FieldType get_##FieldName() const { return FieldName##_; } \
    void set_##FieldName(FieldType value) { FieldName##_ = value; } \
    std::string getFieldValue(const std::string& fieldName) const override { \
        if(fieldName == #FieldName) { return std::to_string(FieldName##_); } \
        return ""; \
    } \
    void setFieldValue(const std::string& fieldName, const std::string& value) override { \
        if(fieldName == #FieldName) { FieldName##_ = std::stoi(value); } \
    } \
    FieldType FieldName##_

// 定义一个可反射类型
class Person : public Reflectable {
    REFLECTABLE_TYPE(Person) {
        REFLECTABLE_FIELD_GETSET(age, int);
        REFLECTABLE_FIELD_GETSET(name, std::string);
    }
};

// 注册可反射类型
int main() {
    ReflectionRegistry::registerType<Person>();

    auto person = ReflectionRegistry::create("Person");
    person->setFieldValue("name", "Tom");
    person->setFieldValue("age", "23");

    person->dump();
    return 0;
}

*/
