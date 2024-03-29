#include <gtest/gtest.h>

#include <functional>
#include <stdexcept>

#include <coroutine/api.hpp>

TEST(Coroutine, CreateAndResume) {
  std::optional<int> yield_value{};
  auto coro = coroutine::Create<int, int, int>(
      std::function<void(int, int)>(
          [](int lhs, int rhs) -> void { coroutine::Yield(lhs - rhs); }),
      5, 3);
  EXPECT_EQ(coro.status, coroutine::Status::kSuspended);

  yield_value = coroutine::Resume(&coro);
  EXPECT_EQ(coro.status, coroutine::Status::kSuspended);
  EXPECT_TRUE(static_cast<bool>(yield_value));
  EXPECT_EQ(*yield_value, 2);

  yield_value = coroutine::Resume(&coro);
  EXPECT_EQ(coro.status, coroutine::Status::kDead);
  EXPECT_FALSE(static_cast<bool>(yield_value));
}

struct TreeNode {
  int val;
  TreeNode *left;
  TreeNode *right;

  explicit TreeNode(int val) : TreeNode(val, nullptr, nullptr) {}
  TreeNode(int val, TreeNode *left, TreeNode *right)
      : val(val), left(left), right(right) {}
};

void TraverseInOrder(TreeNode *node) {
  if (node == nullptr) {
    return;
  }
  TraverseInOrder(node->left);
  coroutine::Yield(node->val);
  TraverseInOrder(node->right);
}

TEST(Coroutine, TraverseBinaryTree1) {
  TreeNode *root = new TreeNode(
      3, new TreeNode(1, nullptr, new TreeNode(2)),
      new TreeNode(8,
                   new TreeNode(5, new TreeNode(4),
                                new TreeNode(7, new TreeNode(6), nullptr)),
                   new TreeNode(9)));
  auto coro = coroutine::Create<int, TreeNode *>(
      std::function<void(TreeNode *)>(TraverseInOrder), root);
  for (int expected = 1; expected <= 9; ++expected) {
    EXPECT_EQ(coro.status, coroutine::Status::kSuspended);
    auto yield_value = coroutine::Resume(&coro);
    EXPECT_EQ(coro.status, coroutine::Status::kSuspended);
    EXPECT_TRUE(static_cast<bool>(yield_value));
    EXPECT_EQ(*yield_value, expected);
  }
  auto yield_value = coroutine::Resume(&coro);
  EXPECT_EQ(coro.status, coroutine::Status::kDead);
  EXPECT_FALSE(static_cast<bool>(yield_value));
}

TEST(Coroutine, TraverseBinaryTree2) {
  TreeNode *root = new TreeNode(
      10,
      new TreeNode(4, new TreeNode(2),
                   new TreeNode(8, new TreeNode(6), nullptr)),
      new TreeNode(18, new TreeNode(14, new TreeNode(12), new TreeNode(16)),
                   new TreeNode(22, new TreeNode(20), nullptr)));
  auto coro = coroutine::Create<int, TreeNode *>(
      std::function<void(TreeNode *)>(TraverseInOrder), root);
  for (int expected = 2; expected <= 22; expected += 2) {
    EXPECT_EQ(coro.status, coroutine::Status::kSuspended);
    auto yield_value = coroutine::Resume(&coro);
    EXPECT_EQ(coro.status, coroutine::Status::kSuspended);
    EXPECT_TRUE(static_cast<bool>(yield_value));
    EXPECT_EQ(*yield_value, expected);
  }
  auto yield_value = coroutine::Resume(&coro);
  EXPECT_EQ(coro.status, coroutine::Status::kDead);
  EXPECT_FALSE(static_cast<bool>(yield_value));
}

void MergeTrees(TreeNode *lhs_root, TreeNode *rhs_root) {
  auto lhs = coroutine::Create<int, TreeNode *>(
      std::function<void(TreeNode *)>(TraverseInOrder), lhs_root);
  auto rhs = coroutine::Create<int, TreeNode *>(
      std::function<void(TreeNode *)>(TraverseInOrder), rhs_root);
  auto lhs_value = coroutine::Resume(&lhs), rhs_value = coroutine::Resume(&rhs);
  while (lhs_value || rhs_value) {
    if (lhs_value && (!rhs_value || *lhs_value < *rhs_value)) {
      coroutine::Yield(*lhs_value);
      lhs_value = coroutine::Resume(&lhs);
    } else {
      coroutine::Yield(*rhs_value);
      rhs_value = coroutine::Resume(&rhs);
    }
  }
}

TEST(Coroutine, MergeBinaryTrees) {
  TreeNode *root1 = new TreeNode(
      5, new TreeNode(1, nullptr, new TreeNode(3)),
      new TreeNode(15,
                   new TreeNode(9, new TreeNode(7),
                                new TreeNode(13, new TreeNode(11), nullptr)),
                   new TreeNode(17)));
  TreeNode *root2 = new TreeNode(
      10,
      new TreeNode(4, new TreeNode(2),
                   new TreeNode(8, new TreeNode(6), nullptr)),
      new TreeNode(18, new TreeNode(14, new TreeNode(12), new TreeNode(16)),
                   new TreeNode(20, new TreeNode(19), nullptr)));
  auto coro = coroutine::Create<int, TreeNode *, TreeNode *>(
      std::function<void(TreeNode *, TreeNode *)>(MergeTrees), root1, root2);
  for (int expected = 1; expected <= 20; expected++) {
    EXPECT_EQ(coro.status, coroutine::Status::kSuspended);
    auto yield_value = coroutine::Resume(&coro);
    EXPECT_EQ(coro.status, coroutine::Status::kSuspended);
    EXPECT_TRUE(static_cast<bool>(yield_value));
    EXPECT_EQ(*yield_value, expected);
  }
  auto yield_value = coroutine::Resume(&coro);
  EXPECT_EQ(coro.status, coroutine::Status::kDead);
  EXPECT_FALSE(static_cast<bool>(yield_value));
}

TEST(Coroutine, IsPolymorphic) {
  std::vector<std::string> input{"The",  "quick", "brown", "fox", "jumps",
                                 "over", "the",   "lazy",  "dog"};
  auto producer =
      coroutine::Create<std::string>(std::function<void()>([&input]() {
        for (const auto &item : input) {
          coroutine::Yield(item);
        }
      }));
  auto transformer =
      coroutine::Create<int>(std::function<void()>([&producer]() {
        std::optional<std::string> yield_value = coroutine::Resume(&producer);
        for (; yield_value; yield_value = coroutine::Resume(&producer)) {
          coroutine::Yield(static_cast<int>(yield_value->size()));
        }
      }));
  std::vector<int> expected{3, 5, 5, 3, 5, 4, 3, 4, 3};
  for (auto expected_value : expected) {
    EXPECT_EQ(transformer.status, coroutine::Status::kSuspended);
    auto yield_value = coroutine::Resume(&transformer);
    EXPECT_EQ(transformer.status, coroutine::Status::kSuspended);
    EXPECT_TRUE(static_cast<bool>(yield_value));
    EXPECT_EQ(*yield_value, expected_value);
  }
  auto yield_value = coroutine::Resume(&transformer);
  EXPECT_EQ(transformer.status, coroutine::Status::kDead);
  EXPECT_FALSE(static_cast<bool>(yield_value));
}

TEST(Coroutine, ExceptionDoesNotTriggerAddressSanitizer) {
  auto bad_coro = coroutine::Create<int>(
      std::function<void()>([]() { throw std::runtime_error{"sike"}; }));
  bool exception_was_caught = false;
  try {
    coroutine::Resume(&bad_coro);
  } catch (std::runtime_error exc) {
    EXPECT_EQ(exc.what(), "sike");
    exception_was_caught = true;
  }
  EXPECT_TRUE(exception_was_caught);
}
