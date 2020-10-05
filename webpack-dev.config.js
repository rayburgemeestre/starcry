const path = require('path');
const webpack = require('webpack');
const MonacoWebpackPlugin = require('monaco-editor-webpack-plugin');
const ExtractTextPlugin = require("extract-text-webpack-plugin");
const { VueLoaderPlugin } = require('vue-loader');
const HtmlWebpackPlugin = require('html-webpack-plugin');

module.exports = {
    mode: 'development', // change to production when ready

    devServer: {
        contentBase: "./webroot",
        hot: true,
        watchOptions: {
            poll: true
        }
    },

    entry: './websrc/index.js',
    output: {
        filename: 'main.js',
        path: path.resolve(__dirname, 'dist')
    },
    module: {
        rules: [
            {
                test: /\.scss$/,
                use: ExtractTextPlugin.extract({
                    fallback: 'style-loader',
                    use: [
                        'css-loader',
                        'sass-loader'
                    ]
                })
            },
            {
                test: /\.css$/,
                use: ['vue-style-loader', 'style-loader', 'css-loader']
            },
            {
                test: /\.vue$/,
                use: 'vue-loader'
            }
        ]
    },
    plugins: [
        new webpack.HotModuleReplacementPlugin(), // for development
        new HtmlWebpackPlugin({
            filename: 'index.html',
            template: 'webroot/index.html',
            inject: true
        }), // for development
        new MonacoWebpackPlugin(),
        new ExtractTextPlugin('css/mystyles.css'),
        new VueLoaderPlugin()
    ]
};

