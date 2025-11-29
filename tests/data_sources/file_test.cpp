#include <gtest/gtest.h>

#include "yapl/detail/data_sources/file.hpp"

#include <array>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <vector>

namespace yapl::data_sources::test {

class FileDataSourceTest : public ::testing::Test {
  protected:
    void SetUp() override {
        // Create a temporary test file
        m_test_file_path =
            std::filesystem::temp_directory_path() / "yapl_test_file.bin";
        std::ofstream file(m_test_file_path, std::ios::binary);
        file.write(reinterpret_cast<const char *>(m_test_data.data()),
                   m_test_data.size());
        file.close();
    }

    void TearDown() override {
        // Clean up test file
        if (std::filesystem::exists(m_test_file_path)) {
            std::filesystem::remove(m_test_file_path);
        }
    }

    std::filesystem::path m_test_file_path;
    std::array<uint8_t, 256> m_test_data = []() {
        std::array<uint8_t, 256> data{};
        for (size_t i = 0; i < data.size(); ++i) {
            data[i] = static_cast<uint8_t>(i);
        }
        return data;
    }();
};

TEST_F(FileDataSourceTest, ConstructorStoresPath) {
    file source(m_test_file_path);
    EXPECT_FALSE(source.is_open());
}

TEST_F(FileDataSourceTest, OpenSucceedsWithValidFile) {
    file source(m_test_file_path);
    ASSERT_NO_THROW(source.open());
    EXPECT_TRUE(source.is_open());
}

TEST_F(FileDataSourceTest, OpenThrowsOnInvalidFile) {
    file source("/nonexistent/path/to/file.bin");
    EXPECT_THROW(source.open(), std::runtime_error);
    EXPECT_FALSE(source.is_open());
}

TEST_F(FileDataSourceTest, CloseClosesFile) {
    file source(m_test_file_path);
    source.open();
    ASSERT_TRUE(source.is_open());

    source.close();
    EXPECT_FALSE(source.is_open());
}

TEST_F(FileDataSourceTest, AvailableReturnsFileSizeAfterOpen) {
    file source(m_test_file_path);
    source.open();
    EXPECT_EQ(source.available(), m_test_data.size());
}

TEST_F(FileDataSourceTest, ReadDataReadsCorrectBytes) {
    file source(m_test_file_path);
    source.open();

    std::vector<uint8_t> buffer(64);
    const size_t bytes_read = source.read_data(64, buffer);

    EXPECT_EQ(bytes_read, 64);
    for (size_t i = 0; i < 64; ++i) {
        EXPECT_EQ(buffer[i], m_test_data[i]) << "Mismatch at index " << i;
    }
}

TEST_F(FileDataSourceTest, ReadDataUpdatesAvailable) {
    file source(m_test_file_path);
    source.open();

    std::vector<uint8_t> buffer(100);
    source.read_data(100, buffer);

    EXPECT_EQ(source.available(), m_test_data.size() - 100);
}

TEST_F(FileDataSourceTest, ReadDataReturnsZeroOnEOF) {
    file source(m_test_file_path);
    source.open();

    // Read entire file
    std::vector<uint8_t> buffer(m_test_data.size());
    source.read_data(m_test_data.size(), buffer);

    // Try to read more
    std::vector<uint8_t> extra_buffer(10);
    const size_t bytes_read = source.read_data(10, extra_buffer);

    EXPECT_EQ(bytes_read, 0);
    EXPECT_EQ(source.available(), 0);
}

TEST_F(FileDataSourceTest, ReadDataClampsToAvailableBytes) {
    file source(m_test_file_path);
    source.open();

    // Read almost everything
    std::vector<uint8_t> buffer(m_test_data.size() - 10);
    source.read_data(buffer.size(), buffer);

    // Request more than available
    std::vector<uint8_t> final_buffer(100);
    const size_t bytes_read = source.read_data(100, final_buffer);

    EXPECT_EQ(bytes_read, 10); // Only 10 bytes were left
}

TEST_F(FileDataSourceTest, ReadDataThrowsOnSmallBuffer) {
    file source(m_test_file_path);
    source.open();

    std::vector<uint8_t> small_buffer(10);
    EXPECT_THROW(source.read_data(100, small_buffer), std::invalid_argument);
}

TEST_F(FileDataSourceTest, ReadDataThrowsWhenNotOpen) {
    file source(m_test_file_path);

    std::vector<uint8_t> buffer(10);
    EXPECT_THROW(source.read_data(10, buffer), std::runtime_error);
}

TEST_F(FileDataSourceTest, ReadDataWithZeroSizeReturnsZero) {
    file source(m_test_file_path);
    source.open();

    std::vector<uint8_t> buffer(10);
    const size_t bytes_read = source.read_data(0, buffer);

    EXPECT_EQ(bytes_read, 0);
    EXPECT_EQ(source.available(), m_test_data.size()); // Position unchanged
}

TEST_F(FileDataSourceTest, ResetResetsPosition) {
    file source(m_test_file_path);
    source.open();

    // Read some data
    std::vector<uint8_t> buffer(100);
    source.read_data(100, buffer);
    ASSERT_EQ(source.available(), m_test_data.size() - 100);

    // Reset
    source.reset();
    EXPECT_EQ(source.available(), m_test_data.size());

    // Verify we can read from the beginning again
    std::vector<uint8_t> new_buffer(64);
    source.read_data(64, new_buffer);
    for (size_t i = 0; i < 64; ++i) {
        EXPECT_EQ(new_buffer[i], m_test_data[i]) << "Mismatch at index " << i;
    }
}

TEST_F(FileDataSourceTest, DestructorClosesFile) {
    auto source = std::make_unique<file>(m_test_file_path);
    source->open();
    ASSERT_TRUE(source->is_open());

    // Destructor should close the file without issues
    EXPECT_NO_THROW(source.reset());
}

TEST_F(FileDataSourceTest, MoveConstructorTransfersOwnership) {
    file source1(m_test_file_path);
    source1.open();
    ASSERT_TRUE(source1.is_open());

    file source2(std::move(source1));
    EXPECT_TRUE(source2.is_open());

    // Verify we can still read
    std::vector<uint8_t> buffer(10);
    EXPECT_NO_THROW(source2.read_data(10, buffer));
}

TEST_F(FileDataSourceTest, MoveAssignmentTransfersOwnership) {
    file source1(m_test_file_path);
    source1.open();

    file source2(m_test_file_path);

    source2 = std::move(source1);
    EXPECT_TRUE(source2.is_open());

    std::vector<uint8_t> buffer(10);
    EXPECT_NO_THROW(source2.read_data(10, buffer));
}

TEST_F(FileDataSourceTest, SequentialReadsAreCorrect) {
    file source(m_test_file_path);
    source.open();

    // Read in chunks and verify sequential data
    std::vector<uint8_t> buffer(32);
    size_t total_read = 0;

    for (int chunk = 0; chunk < 8; ++chunk) {
        const size_t bytes_read = source.read_data(32, buffer);
        ASSERT_EQ(bytes_read, 32);

        for (size_t i = 0; i < 32; ++i) {
            EXPECT_EQ(buffer[i], m_test_data[total_read + i])
                << "Mismatch at chunk " << chunk << ", index " << i;
        }
        total_read += bytes_read;
    }

    EXPECT_EQ(total_read, m_test_data.size());
    EXPECT_EQ(source.available(), 0);
}

TEST_F(FileDataSourceTest, EmptyFileReturnsZeroAvailable) {
    // Create an empty test file
    const auto empty_file_path =
        std::filesystem::temp_directory_path() / "yapl_empty_test.bin";
    std::ofstream empty_file(empty_file_path, std::ios::binary);
    empty_file.close();

    file source(empty_file_path);
    source.open();

    EXPECT_EQ(source.available(), 0);

    std::vector<uint8_t> buffer(10);
    EXPECT_EQ(source.read_data(10, buffer), 0);

    std::filesystem::remove(empty_file_path);
}

} // namespace yapl::data_sources::test
