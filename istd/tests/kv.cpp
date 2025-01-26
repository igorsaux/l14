// Space Station 14 Launcher
// Copyright (C) 2025 Igor Spichkin
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <catch2/catch_all.hpp>

import istd.format.kv;

using namespace istd;
using namespace istd::format;

using kv::Array;
using kv::Object;
using kv::Value;

TEST_CASE( "Values", "[kv]" )
{
    const auto v1 = Value{ "Hello, world!" };
    const auto v2 = Value{ "Hello, world!" };
    const auto v3 = Value{ "Test" };
    const auto v4 = Value{ 10 };
    const auto v5 = Value::FromBool( true );
    const auto v6 = Value::FromBool( false );
    const Value v7 = Object{
        { "string_value", "Test" },
        { "state", Value::FromBool( true ) },
    };
    const Value v8 = Object{
        { "string_value", "Test" },
        { "state", Value::FromBool( true ) },
    };
    const Value v9 = Object{
        { "foo", Value{ 12 } },
    };

    REQUIRE( v1.IsString() );
    REQUIRE( v1.GetString() == "Hello, world!" );
    REQUIRE( v2.IsString() );
    REQUIRE( v1 == v2 );
    REQUIRE( v3.GetString() == "Test" );
    REQUIRE( v1 != v3 );
    REQUIRE( v4.IsNumber() );
    REQUIRE( v4.GetNumber() == 10 );
    REQUIRE( v5.IsBool() );
    REQUIRE( v5.GetBool() == true );
    REQUIRE( v6.IsBool() );
    REQUIRE( v6.GetBool() == false );
    REQUIRE( v5 != v6 );
    REQUIRE( v7.IsObject() );
    REQUIRE( v8.IsObject() );
    REQUIRE( v7 == v8 );
    REQUIRE( v7 != v9 );
}

TEST_CASE( "Parsing", "[kv]" )
{
    constexpr std::string_view data = R"(
// Comment
"StringValue" "Hello, world!"
"FloatValue1" 123.23
"FloatValue2" 0.05
"NestedArrays" [
  [1 2]
  [3 4]
  []
]
"IntValue" 50
"ArrayValue" [
  25
  "Test"
  // Another comment
  {
    "Key" "Value"
  }
]
"TestArray" [false "Foo" 55] // Yet another comment
"BooleanValue" true
"EmptyArray" []
"ObjectValue" {
  "Foo" "Bar"
}
"EmptyObject" {
}
)";

    const auto [object, error] = format::kv::Parse( data );

    REQUIRE( !error.has_value() );

    const auto expected = Value{
        Object{
            { "StringValue", "Hello, world!" },
            { "FloatValue1", 123.23 },
            { "FloatValue2", 0.05 },
            { "IntValue", 50 },
            { "TestArray", Array{ Value::FromBool( false ), "Foo", 55 } },
            { "NestedArrays", Array{ Array{ 1, 2 }, Array{ 3, 4 }, Array{} } },
            { "BooleanValue", Value::FromBool( true ) },
            { "ArrayValue", Array{ 25, "Test", Object{ { "Key", "Value" } } } },
            { "EmptyArray", Array{} },
            { "ObjectValue", Object{ { "Foo", "Bar" } } },
            { "EmptyObject", Object{} } },
    };

    REQUIRE( expected == object );
}

TEST_CASE( "Merging", "[kv]" )
{
    constexpr std::string_view data = R"(
"Foo" {
    "IntValue1" 65
    "BoolValue1" false
    "ObjectValue" {
        "ArrayValue" [1 2 3]
    }
}

"Foo" {
    "BoolValue1" true
    "IntValue2" 5
    "ObjectValue" {
        "BoolValue2" false
    }
}
)";

    const auto [object, error] = format::kv::Parse( data );

    REQUIRE( !error.has_value() );

    const auto expected = Value{
        Object{
            {
                "Foo",
                Object{
                    { "IntValue1", 65.0 },
                    { "IntValue2", 5.0 },
                    { "BoolValue1", Value::FromBool( true ) },
                    {
                        "ObjectValue",
                        Object{ { "ArrayValue", Array{ 1.0, 2.0, 3.0 } },
                                { "BoolValue2", Value::FromBool( false ) } },
                    },
                },
            },
        },
    };

    REQUIRE( object == expected );
}

TEST_CASE( "Serialize & Deserialize", "[kv]" )
{
    const auto expected = Value{
        Object{
            { "StringValue", "Hello, world!" },
            { "FloatValue1", 123.23 },
            { "FloatValue2", 0.05 },
            { "IntValue", 50 },
            { "TestArray", Array{ Value::FromBool( false ), "Foo", 55 } },
            { "NestedArrays", Array{ Array{ 1, 2 }, Array{ 3, 4 }, Array{} } },
            { "BooleanValue", Value::FromBool( true ) },
            { "ArrayValue", Array{ 25, "Test", Object{ { "Key", "Value" } } } },
            { "EmptyArray", Array{} },
            { "ObjectValue", Object{ { "Foo", "Bar" } } },
            { "EmptyObject", Object{} } },
    };

    const auto serialized = kv::Serialize( expected.GetObject() );
    const auto text = kv::ToString( serialized );

    const auto [data, error] = kv::Parse( text );

    REQUIRE( !error.has_value() );
    REQUIRE( data == expected );
}
