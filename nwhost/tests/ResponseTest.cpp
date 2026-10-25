// ResponseTest.cpp

#include "Response.hh"

#include <gtest/gtest.h>

using nwhost::Response;

TEST(Response, HeaderIsFFAF05)
{
    // the leading FF is there because the first read of the MSX might fail (hardware design choice)
    Response response;
    response.sendHeader();
    EXPECT_EQ(response.getSize(), 3u);
    EXPECT_EQ(response.read(), 0xff);
    EXPECT_EQ(response.read(), 0xaf);
    EXPECT_EQ(response.read(), 0x05);
}

TEST(Response, Send16IsLittleEndian)
{
    Response response;
    response.send16(0x1234);
    EXPECT_EQ(response.read(), 0x34);
    EXPECT_EQ(response.read(), 0x12);
}

TEST(Response, ReadingAnEmptyResponseGivesFF)
{
    // what the MSX reads from an empty fifo
    Response response;
    EXPECT_FALSE(response.isDataAvailable());
    EXPECT_EQ(response.read(), 0xff);
    EXPECT_EQ(response.peek(), 0xff);
}

TEST(Response, PeekDoesNotConsume)
{
    Response response;
    response.send(7);
    EXPECT_EQ(response.peek(), 7);
    EXPECT_EQ(response.peek(), 7);
    EXPECT_EQ(response.read(), 7);
    EXPECT_FALSE(response.isDataAvailable());
}

TEST(Response, PurgeDropsEverything)
{
    Response response;
    response.sendHeader();
    response.send(1);
    response.purge();
    EXPECT_FALSE(response.isDataAvailable());
    EXPECT_EQ(response.getSize(), 0u);
}
