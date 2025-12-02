#include <cstddef>
#include <iostream>
#include <ostream>
#include <vector>

#ifndef SPARSE_HPP

    class ISparseSet {
    public:
        virtual ~ISparseSet() = default;
        virtual void removeId(std::size_t Entity) = 0;
        virtual bool has(std::size_t Entity) const = 0;
    };

    template<typename data_type>
    class SparseSet : public ISparseSet {
        private:

            /** 
                A vector containing the index of the data in the dense vector at the id position.
                The index equal -1 if their is no data
            */
            std::vector<int> _sparse;

            /** A vector containing the data stored contigously */
            std::vector<data_type> _dense;

            /**
                A vector containing id at their data index positions.
                The id are stored contigously
            */
            std::vector<std::size_t> _reverse_dense;
        public:
            /**
                A function to add a new data to the given id
                @param std::size_t id
            */
            void addID(std::size_t id, const data_type& data)
            {
                if (id >= _sparse.size()) {
                    _sparse.resize(id + 1, -1);
                }
                if (_sparse[id] != -1) {
                    _dense[_sparse[id]] = data;
                    return;
                }
                _sparse[id] = _dense.size();
                _dense.push_back(data);
                _reverse_dense.push_back(id);
                return;
            }

            /**
                A function to remove a data from the given id
                @param std::size_t id
            */
            void removeId(std::size_t id) override
            {
                if (!has(id)) {
                    std::cerr << "Error: removeId: " << id 
                    << " does not have any components from this type." << std::endl;
                    return;
                }
                std::size_t indexToRemove = _sparse[id];
                std::size_t lastIndex = _dense.size() - 1;
                if (indexToRemove != lastIndex) {
                    data_type lastData = _dense[lastIndex];
                    std::size_t lastEntity = _reverse_dense[lastIndex];

                    _dense[indexToRemove] = lastData;
                    _reverse_dense[indexToRemove] = lastEntity;
                    _sparse[lastIndex] = indexToRemove;
                }
                _sparse[id] = -1;
                _dense.pop_back();
                _reverse_dense.pop_back();
                std::cout << "Component from entity " << id << " successfully removed." << std::endl;
                return;
            }

            /**
                A function to check if an id has a data
            */
            bool has(std::size_t id) const override
            {
                if (_sparse[id] > -1 && id < _sparse.size())
                    return true;
                return false;
            }

            /**
                A function to get the data of a given id
                @param std::size_t id
                @return The function returns reference to the corresponding data
            */
            data_type& getDataFromId(std::size_t id)
            {
                return _dense[_sparse[id]];
            }

            /**
                A function to get the id of a given data
                @param data_type data
                @return The function returns the corresponding id 
            */
            std::size_t getIdFromData(data_type data) const
            {
                return _reverse_dense[data];
            }

            /**
                A function to get the data's list stored
                @return The function returns the dense vector
            */
            std::vector<data_type>& getDataList()
            {
                return _dense;
            }

            /**
                A function to get the id's list stored
                @return The function returns the _reverse_dense vector
            */
            std::vector<std::size_t>& getIdList()
            {
                return _reverse_dense;
            }
    };

#endif