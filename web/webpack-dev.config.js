const path = require('path');
const webpack = require('webpack');
const MonacoWebpackPlugin = require('monaco-editor-webpack-plugin');
const MiniCssExtractPlugin = require('mini-css-extract-plugin');
const {VueLoaderPlugin} = require('vue-loader');
const HtmlWebpackPlugin = require('html-webpack-plugin');

module.exports = {
  mode: 'development',

  devServer: {
    static: './webroot',
    hot: true,
  },

  entry: './websrc/index.js',
  output: {filename: 'main.js', path: path.resolve(__dirname, 'webroot')},
  resolve: {
    alias: {
      vue: '@vue/compat'
    }
  },
  module: {
    rules: [
      {
        test: /\.(sa|sc|c)ss$/,
        use: [
          MiniCssExtractPlugin.loader,
          'css-loader',
          'sass-loader',
        ],
      },
      {
        test: /\.vue$/,
        loader: 'vue-loader',
        options: {
          compilerOptions: {
            compatConfig: {
              MODE: 2,
            },
          },
        },
      },
      {
        test: /\.ttf$/,
        use: ['file-loader'],
      }
    ]
  },
  plugins: [
    new webpack.HotModuleReplacementPlugin(),                                                       // for development
    new HtmlWebpackPlugin({filename: 'index.html', template: 'webroot/index.html', inject: true}),  // for development
    new MonacoWebpackPlugin(),
    new MiniCssExtractPlugin(),
    new VueLoaderPlugin()
  ]
};
