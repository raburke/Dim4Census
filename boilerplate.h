
#include <fstream>
#include <map>
#include <string>
#include "triangulation/dim4.h"
#include "triangulation/isosigtype.h"
#include "triangulation/detail/isosig-impl.h"

class Node;

using SigMap = std::map<std::string, Node*>;

// Union-find support:
class Node {
    private:
        // Union-find parent node:
        Node* parent_ { nullptr };

        // Data that is only maintained for the component root:
        size_t depth_ { 0 };
        size_t size_ { 1 };
        Node* prevComp_ { nullptr }; // root node for previous component
        Node* nextComp_ { nullptr }; // root node for next component
        SigMap::iterator rep_; // preferred representative triangulation

    public:
        Node() = default;

        /**
         * Find the root node (i.e., the "canonical" representative) for the
         * component containing this node.
         */
        Node* root() {
            if (! parent_)
                return this;

            // This is a non-root node.

            // Find the root.
            Node* root = parent_;
            while (root->parent_)
                root = root->parent_;

            // Compress the tree - adjust all nodes on the path from this node
            // to the root so that their parents point directly to the root.
            Node* n = this;
            while (n->parent_ != root) {
                Node* tmp = n->parent_;
                n->parent_ = root;
                n = tmp;
            }

            return root;
        }

        // Make nodes non-copyable.
        Node(const Node&) = delete;
        Node& operator = (const Node&) = delete;

    friend class Component;
    friend class TriangulationSet;
};

// Lightweight class, cheap and safe to copy
class Component {
    private:
        /**
         * This may be any node within the component (since merges could
         * turn roots into non-roots and we do not want to have to update
         * existing Component objects to account for this).
         *
         * This is null for a past-the-end component (for iteration).
         */
        Node* comp_ { nullptr };

    private:
        /**
         * Update comp_ to ensure it is pointing to a root node.
         * This is marked as const since it doesn't actually change the
         * component, it merely updates the internal representation.
         *
         * PRE: This is not a past-the-end component.
         */
        void makeRoot() const {
            while (comp_->parent_)
                const_cast<Component*>(this)->comp_ = comp_->parent_;
        }

    public:
        Component() = default;
        Component(Node* comp) : comp_(comp) {}
        Component& operator = (const Component&) = default;

        /**
         * Returns true iff this is not a past-the-end component.
         */
        operator bool() const {
            return comp_;
        }

        /**
         * Determines whether this and the given object point to the same
         * component.  Past-the-end components are allowed.
         */
        bool operator == (const Component& other) const {
            if (comp_)
                makeRoot();
            if (other.comp_)
                other.makeRoot();
            return (comp_ == other.comp_);
        }

        /**
         * Determines whether this and the given object do not point to the
         * same component.  Past-the-end components are allowed.
         */
        bool operator != (const Component& other) const {
            if (comp_)
                makeRoot();
            if (other.comp_)
                other.makeRoot();
            return (comp_ != other.comp_);
        }

        /**
         * Returns the number of triangulations in this component.
         *
         * PRE: This is not a past-the-end component.
         */
        size_t size() const {
            makeRoot();
            return comp_->size_;
        }

        /**
         * Returns the "canonical" representative for this component.
         *
         * PRE: This is not a past-the-end component.
         */
        regina::Triangulation<4> rep() const {
            makeRoot();
            return regina::Triangulation<4>::fromIsoSig(comp_->rep_->first);
        }

        /**
         * Preincrement operator that advances this to point to the
         * next component.
         *
         * PRE: This is not a past-the-end component.
         */
        Component& operator ++ () {
            makeRoot();
            comp_ = comp_->nextComp_;
            return *this;
        }

        /**
         * Postincrement operator that advances this to point to the
         * next component.
         *
         * PRE: This is not a past-the-end component.
         */
        Component operator ++ (int) {
            makeRoot();
            Component ans(comp_);
            comp_ = comp_->nextComp_;
            return ans;
        }

        /**
         * Preincrement operator that "reverse advances" this to point to the
         * previous component.
         *
         * PRE: This is not a past-the-end component.
         */
        Component& operator -- () {
            makeRoot();
            comp_ = comp_->prevComp_;
            return *this;
        }

        /**
         * Postincrement operator that "reverse advances" this to point to the
         * previous component.
         *
         * PRE: This is not a past-the-end component.
         */
        Component operator -- (int) {
            makeRoot();
            Component ans(comp_);
            comp_ = comp_->prevComp_;
            return ans;
        }

    friend class TriangulationSet;
};

// Heavyweight class, do not copy!
class TriangulationSet {
    private:
        SigMap nodes_;
        size_t components_ { 0 };
        Node* firstComp_ { nullptr };
        Node* lastComp_ { nullptr };

    private:
        class iterator {
            private:
                SigMap::const_iterator mapIt_;

            public:
                iterator() = default;
                iterator(const iterator&) = default;
                iterator(SigMap::const_iterator mapIt) : mapIt_(mapIt) {}

                iterator& operator = (const iterator&) = default;

                bool operator == (const iterator& rhs) const {
                    return mapIt_ == rhs.mapIt_;
                }

                bool operator != (const iterator& rhs) const {
                    return mapIt_ != rhs.mapIt_;
                }

                regina::Triangulation<4> operator * () const {
                    return regina::Triangulation<4>::fromIsoSig(mapIt_->first);
                }

                iterator& operator ++ () {
                    ++mapIt_;
                    return *this;
                }

                iterator operator ++ (int) {
                    iterator ans = *this;
                    ++mapIt_;
                    return ans;
                }

                iterator& operator -- () {
                    --mapIt_;
                    return *this;
                }

                iterator operator -- (int) {
                    iterator ans = *this;
                    --mapIt_;
                    return ans;
                }
        };

    private:
        /**
         * Inserts a the given isosig into the set, as an isolated component.
         *
         * PRE: The given isosig is not already in the set.
         */
        Node* createNode(std::string&& sig) {
            Node* n = new Node();
            n->rep_ = nodes_.emplace(std::move(sig), n).first;

            n->prevComp_ = lastComp_;
            if (lastComp_)
                lastComp_->nextComp_ = n;
            else
                firstComp_ = n;
            lastComp_ = n;

            ++components_;

            return n;
        }

        /**
         * Returns the node corresponding to the given triangulation.
         * If the triangulation is not already in the set, it will be inserted
         * as a new isolated component.
         */
        Node* node(const regina::Triangulation<4>& tri) {
            auto sig = tri.isoSig<regina::IsoSigEdgeDegrees<4>>();
            auto pos = nodes_.find(sig);
            if (pos == nodes_.end())
                return createNode(std::move(sig));
            else
                return pos->second;
        }

        /**
         * Merge the components containing the two given nodes.
         * These do not need to be root nodes.
         *
         * Returns true if two distinct components were merged, or false if
         * both nodes already belonged to the same component.
         *
         * If a merge does take place:
         *
         * - the merged component will occupy the place in the linked list
         *   that was previously occupied by n1;
         *
         * - the merged component will use the preferred representative from n1
         *   if useRep1 is true, or from n2 if useRep1 is false.
         */
        bool merge(Node* n1, Node* n2, bool useRep1) {
            n1 = n1->root();
            n2 = n2->root();
            if (n1 == n2)
                return false;

            if (n1->depth_ > n2->depth_) {
                // Make n1 the root for both components.
                n2->parent_ = n1;
                n1->size_ += n2->size_;

                // Remove the old root n2 from the linked list of components.
                if (n2->prevComp_)
                    n2->prevComp_->nextComp_ = n2->nextComp_;
                else
                    firstComp_ = n2->nextComp_;
                if (n2->nextComp_)
                    n2->nextComp_->prevComp_ = n2->prevComp_;
                else
                    lastComp_ = n2->prevComp_;

                if (! useRep1)
                    n1->rep_ = n2->rep_;
            } else {
                // Make n2 the root for both components.
                n1->parent_ = n2;
                n2->size_ += n1->size_;
                if (n1->depth_ == n2->depth_)
                    ++n2->depth_;

                // Move n2 into the place previously occupied by n1 in the
                // linked list of components, and remove n1 completely.
                // We should be careful about how we do this, since n1 and n2
                // could be adjacent in the list.

                // First, remove n2 from the list:
                if (n2->prevComp_)
                    n2->prevComp_->nextComp_ = n2->nextComp_;
                else
                    firstComp_ = n2->nextComp_;
                if (n2->nextComp_)
                    n2->nextComp_->prevComp_ = n2->prevComp_;
                else
                    lastComp_ = n2->prevComp_;

                // Next, adjust the neighbours of n1 to point to n2 instead
                // (and vice versa).
                if (n1->prevComp_) {
                    n1->prevComp_->nextComp_ = n2;
                    n2->prevComp_ = n1->prevComp_;
                } else {
                    firstComp_ = n2;
                    n2->prevComp_ = nullptr;
                }

                if (n1->nextComp_) {
                    n1->nextComp_->prevComp_ = n2;
                    n2->nextComp_ = n1->nextComp_;
                } else {
                    lastComp_ = n2;
                    n2->nextComp_ = nullptr;
                }

                if (useRep1)
                    n2->rep_ = n1->rep_;
            }

            --components_;
            return true;
        }

    public:
        /**
         * Reads in a triangulation set from the given file.
         *
         * The file should be a text file filled with edge-degree isosigs
         * (not classic isosigs), and nothing else.  Isosigs should be
         * separated by whitespace.
         *
         * Each triangulation will become an isolated component.
         *
         * This routine should throw an exception if the file is unreadable.
         * If some piece of text is not a valid isosig, an exception will most
         * likely be thrown but possibly not until much later (when the isosig
         * is actually fleshed out into a real triangulation).
         */
        TriangulationSet(const char* filename) {
            std::ifstream f(filename);

            while (true) {
                std::string sig;
                f >> sig;
                if (! f)
                    break;
                if (sig.size() > 0) {
                    createNode(std::move(sig));

                    // Old code that converted classic isosigs to edge degree
                    // isosigs:
                    // auto tri = regina::Triangulation<4>::fromIsoSig(sig);
                    // createNode(tri.isoSig<regina::IsoSigEdgeDegrees<4>>());
                }
            }
        }

        /**
         * Merge the components containing the two given triangulations.
         *
         * Returns true if two distinct components were merged, or false if
         * both triangulations already belonged to the same component.
         *
         * If a merge does take place:
         *
         * - the merged component will occupy the place in the linked list
         *   that was previously occupied by t1;
         *
         * - the merged component will use the preferred representative from t1
         *   if useRep1 is true (the default), or from t2 if useRep1 is false.
         */
        bool merge(const regina::Triangulation<4>& t1,
                const regina::Triangulation<4>& t2,
                bool useRep1 = true) {
            return merge(node(t1), node(t2), useRep1);
        }

        /**
         * A variant of merge() that takes the component containing the first
         * triangulation, instead of the triangulation itself.  This is faster,
         * in that it avoids an isosig computation and a dictionary lookup.
         *
         * PRE: c is not a past-the-end component.
         */
        bool merge(Component c, const regina::Triangulation<4>& t2,
                bool useRep1 = true) {
            return merge(c.comp_, node(t2), useRep1);
        }

        /**
         * Returns the number of components in the set.
         */
        size_t countComponents() const {
            return components_;
        }

        /**
         * Returns the number of isosigs in the set.
         */
        size_t size() const {
            return nodes_.size();
        }

        Component components() const {
            return firstComp_;
        }

        iterator begin() const {
            return nodes_.begin();
        }

        iterator end() const {
            return nodes_.end();
        }

        TriangulationSet(const TriangulationSet&) = delete;
        TriangulationSet& operator = (const TriangulationSet&) = delete;
};

