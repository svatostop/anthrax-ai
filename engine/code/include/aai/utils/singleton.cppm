export module aai.utils;

export {
    namespace utils {
        template <typename T>
        class singleton
        {
            private:
            protected:
                singleton() {}

            public:
                singleton(const singleton* obj) = delete;
                singleton* operator=(const singleton*) = delete;

                static T* get() {  static T Instance; return &Instance; }
        };

    }
};
