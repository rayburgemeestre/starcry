const path = require('path');
const webpack = require('webpack');
const MonacoWebpackPlugin = require('monaco-editor-webpack-plugin');
const MiniCssExtractPlugin = require("mini-css-extract-plugin");
const { VueLoaderPlugin } = require('vue-loader');
const HtmlWebpackPlugin = require('html-webpack-plugin');

module.exports = {
    mode: 'production',
    entry: './websrc/index.js',
    output: {
        filename: 'main.js',
        path: path.resolve(__dirname, 'webroot')
    },
    module: {
        rules: [
            {
				test: /\.s[ac]ss$/i,
				use: [
					"style-loader",
                     {
                         loader: MiniCssExtractPlugin.loader,
                         options: {
                             esModule: false,
                         },
                     },
					"css-loader",
					"sass-loader",
				],
            },
            {
                test: /\.css$/,
                use: ['style-loader',
                    {
                        loader: MiniCssExtractPlugin.loader,
                        options: {
                            esModule: false,
                        },
                    },
                    'css-loader']
            },
            {
                test: /\.vue$/,
                use: 'vue-loader'
            },
            {
                test: /\.ttf$/,
                use: ['file-loader'],
            },
        ]
    },
    plugins: [
        new MonacoWebpackPlugin(),
        new MiniCssExtractPlugin(),
        new VueLoaderPlugin()
    ]
};

