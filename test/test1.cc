// vim:ts=4:sts=4:sw=4:expandtab

// (c) Bharanee Rathna 2012

#include "beanstalk.h"
#include "gtest/gtest.h"

int handle;

TEST(Connection, Connect) {
    handle = bs_connect((char*)"127.0.0.1", 11300);
    EXPECT_GT(handle, 0);
    EXPECT_EQ(bs_disconnect(handle), BS_STATUS_OK);
    handle = bs_connect((char*)"127.0.0.1", 1);
    EXPECT_LT(handle, 0);
}

TEST(TUBE, USE) {
    handle = bs_connect((char*)"127.0.0.1", 11300);
    EXPECT_GT(handle, 0);
    EXPECT_EQ(bs_use(handle, (char*)"test"), BS_STATUS_OK);
    bs_disconnect(handle);
}

TEST(TUBE, WATCH) {
    handle = bs_connect((char*)"127.0.0.1", 11300);
    EXPECT_GT(handle, 0);
    EXPECT_EQ(bs_watch(handle, (char*)"test"), BS_STATUS_OK);
    bs_disconnect(handle);
}

TEST(TUBE, IGNORE) {
    handle = bs_connect((char*)"127.0.0.1", 11300);
    EXPECT_GT(handle, 0);
    EXPECT_EQ(bs_watch(handle,  (char*)"test"),    BS_STATUS_OK);
    EXPECT_EQ(bs_ignore(handle, (char*)"default"), BS_STATUS_OK);
    EXPECT_EQ(bs_ignore(handle, (char*)"test"),    BS_STATUS_FAIL);
    bs_disconnect(handle);
}

TEST(JOB, PUT) {
    handle = bs_connect((char*)"127.0.0.1", 11300);
    EXPECT_GT(handle, 0);
    EXPECT_EQ(bs_use(handle, (char*)"test"), BS_STATUS_OK);
    EXPECT_GT(bs_put(handle, 1, 0, 1, (char*)"hello", 5), 0);
    bs_disconnect(handle);
}

TEST(JOB, RESERVE_RELEASE_DELETE) {
    BSJ *job;
    handle = bs_connect((char*)"127.0.0.1", 11300);
    EXPECT_GT(handle, 0);
    EXPECT_EQ(bs_watch(handle, (char*)"test"), BS_STATUS_OK);
    EXPECT_EQ(bs_reserve(handle, &job), BS_STATUS_OK);
    EXPECT_EQ(std::string(job->data, job->size), "hello");
    EXPECT_EQ(bs_release(handle, job->id, 1, 0), BS_STATUS_OK);
    EXPECT_EQ(bs_delete(handle, job->id), BS_STATUS_OK);
    bs_free_job(job);
    bs_disconnect(handle);
}

TEST(JOB, LARGE_CHUNK) {
    BSJ *job;
    int lines = 200, bytes = lines * 27 + 1;
    char *message = (char*)calloc(1, bytes);

    // setup message
    for (int i = 0, n = 0; i < lines; i++) {
        for (int j = 0; j < 26; j++)
            message[n++] = j + 97;
        message[n++] = '\n';
    }

    handle = bs_connect((char*)"127.0.0.1", 11300);
    EXPECT_GT(handle, 0);
    EXPECT_EQ(bs_use(handle,   (char*)"test"), BS_STATUS_OK);
    EXPECT_EQ(bs_watch(handle, (char*)"test"), BS_STATUS_OK);
    EXPECT_GT(bs_put(handle, 1, 0, 1, message, bytes), 0);
    EXPECT_EQ(bs_reserve(handle, &job), BS_STATUS_OK);
    EXPECT_EQ(job->size, bytes);
    EXPECT_EQ(memcmp(job->data, message, bytes), 0);
    EXPECT_EQ(bs_delete(handle, job->id), BS_STATUS_OK);

    free(message);
    bs_free_job(job);
    bs_disconnect(handle);
}

int main(int argc, char *argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
