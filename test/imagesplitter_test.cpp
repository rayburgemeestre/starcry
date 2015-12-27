#include "gtest/gtest.h"

#include "util/image_splitter.hpp"

using namespace std;

TEST(ImageSplitterTest, test_split_auto) {
    ImageSplitter<uint32_t> imagesplitter{800, 600};
    const auto rectangles = imagesplitter.split(2);
    // two vertical slices
    ASSERT_EQ(0, rectangles[0].x());
    ASSERT_EQ(0, rectangles[0].y());
    ASSERT_EQ(400, rectangles[0].x2());
    ASSERT_EQ(600, rectangles[0].y2());
    ASSERT_EQ(400, rectangles[1].x());
    ASSERT_EQ(0, rectangles[1].y());
    ASSERT_EQ(800, rectangles[1].x2());
    ASSERT_EQ(600, rectangles[1].y2());
}

TEST(ImageSplitterTest, test_split_auto_2) {
    ImageSplitter<uint32_t> imagesplitter{600, 800};
    const auto rectangles = imagesplitter.split(2);
    // two horizontal slices
    ASSERT_EQ(0, rectangles[0].x());
    ASSERT_EQ(0, rectangles[0].y());
    ASSERT_EQ(800, rectangles[0].x2());
    ASSERT_EQ(400, rectangles[0].y2());
    ASSERT_EQ(0, rectangles[1].x());
    ASSERT_EQ(400, rectangles[1].y());
    ASSERT_EQ(600, rectangles[1].x2());
    ASSERT_EQ(800, rectangles[1].y2());
}

TEST(ImageSplitterTest, test_split_vertical) {
    ImageSplitter<uint32_t> imagesplitter{600, 800};
    const auto rectangles = imagesplitter.split(2, ImageSplitter<uint32_t>::Mode::SplitVertical);
    // two vertical slices
    ASSERT_EQ(0, rectangles[0].x());
    ASSERT_EQ(0, rectangles[0].y());
    ASSERT_EQ(300, rectangles[0].x2());
    ASSERT_EQ(800, rectangles[0].y2());
    ASSERT_EQ(300, rectangles[1].x());
    ASSERT_EQ(0, rectangles[1].y());
    ASSERT_EQ(600, rectangles[1].x2());
    ASSERT_EQ(800, rectangles[1].y2());
}
