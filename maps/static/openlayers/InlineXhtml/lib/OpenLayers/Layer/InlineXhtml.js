/* Copyright (c) 2006-2011 by OpenLayers Contributors (see authors.txt for
 * full list of contributors). Published under the Clear BSD license.
 * See http://svn.openlayers.org/trunk/openlayers/license.txt for the
 * full text of the license. */


/**
 * @requires OpenLayers/Layer/Grid.js
 * @requires OpenLayers/Tile/InlineXhtml.js
 */

/**
 * Class: OpenLayers.Layer.InlineXhtml
 * Instances of OpenLayers.Layer.InlineXhtml are used to retrieve xhtml data.
 * Create a new layer with the <OpenLayers.Layer.InlineXhtml> constructor.
 *
 * Inherits from:
 *  - <OpenLayers.Layer.Grid>
 */
OpenLayers.Layer.InlineXhtml = OpenLayers.Class(OpenLayers.Layer.Grid, {

    /**
    * Property: isHtmlLayer
    * {boolean} If true, the incomming content will be treated as html
    * and the browsers html parser will be used.  If false (default),
    * the content will first be treated (parsed) as xhtml.
    * Applied to Layer tiles.
    * Defaults to; false.
    */ 
    isHtmlLayer: false,

    /**
    * Property: fallbackToHtmlLayer
    * {boolean} If true (and isHtmlLayer is false), the 
    * incomming content will be treated as html should the
    * initial xhtml parsing fail.  Only applies when isHtmlLayer is
    * false.  Applied to Layer tiles
    * Defaults to; false.
    */
    fallbackToHtmlLayer: false,

    /**
    * Property: overflow
    * {DOMElement} Overflow style for layer tiles.
    * Applied to Layer tiles.
    *Defauls to: "hidden".
    */ 
    overflow: 'hidden',

    /**
    * Property: pointerEvents
    * {DOMElement} Pointer-Events style for layer.
    * Applied to Layer.
    */ 
    pointerEvents: null,

    /**
    * Property: xhtmlContainerId
    * {string} String used to search xhtml content for a matching
    * element having the specified id attribute.  Only child nodes of that
    * element will be imported into the tile.
    * Applied to Layer tiles.
    */ 
    xhtmlContainerId: null,

    /**
    * Property: sendIdParams
    * {boolean} indicates whether map, layer and tile Id
    * parameters are to be passed as the url request.
    * Applied to Layer tiles.
    */ 
    sendIdParams: false,

    /**
    * Property: evaluatedParams
    * {object} hash table of parameters to be passed as
    * part of the url request.  Parameters are evaluated
    * prior to appending to request.  Applied to Layer tiles.
    */ 
    evaluatedParams: null,

    /**
     * Constructor: OpenLayers.Layer.InlineXhtml
     * Creates a new InlineXhtml layer object.
     *
     * Parameters:
     * name - {String} A name for the layer
     * url - {String} Base url for the xhtml feed
     * params - {Object} An object with key/value pairs representing the
     *                   query string parameters and parameter values.
     * options - {Object} Hashtable of extra options to tag onto the layer.
     */
    initialize: function(name, url, params, options) {
        OpenLayers.Layer.Grid.prototype.initialize.apply(this, arguments);

        if (options.id == null && this.options.pointerEvents) {
           this.div.style.pointerEvents = this.options.pointerEvents;
        }
    },

    /**
     * Method: getURL
     * Return a query string for this layer
     *
     * Parameters:
     * bounds - {<OpenLayers.Bounds>} A bounds representing the bbox for the
     *                                request.
     *
     * Returns:
     * {String} A string with the layer's url and parameters and also the
     *          passed-in bounds and appropriate tile size specified as 
     *          parameters.
     */
    getURL: function (bounds) {

        var newParams = {};

        bounds = this.adjustBounds(bounds);
        newParams.BBOX = bounds.toBBOX();

        var imageSize = this.getImageSize();
        newParams.WIDTH = imageSize.w;
        newParams.HEIGHT = imageSize.h;

        var requestString = this.getFullRequestString(newParams);
        return requestString;
    },

   /**
     * APIMethod: mergeNewParams
     * Catch changeParams and uppercase the new params to be merged in
     *     before calling changeParams on the super class.
     * 
     *     Once params have been changed, the tiles will be reloaded with
     *     the new parameters.
     * 
     * Parameters:
     * newParams - {Object} Hashtable of new params to use
     */
    mergeNewParams:function(newParams) {
        var upperParams = OpenLayers.Util.upperCaseObject(newParams);
        var newArguments = [upperParams];
        return OpenLayers.Layer.Grid.prototype.mergeNewParams.apply(this, 
                                                             newArguments);
    },

    /** 
     * APIMethod: getFullRequestString
     * Combine the layer's url with its params and these newParams. 
     *   
     *     Add the PROJECTION parameter from projection -- this is probably
     *     more eloquently done via a setProjection() method, but this 
     *     works for now and always.
     *
     * Parameters:
     * newParams - {Object}
     * altUrl - {String} Use this as the url instead of the layer's url
     * 
     * Returns:
     * {String} 
     */
    getFullRequestString:function(newParams, altUrl) {
        var projectionCode = this.map.getProjection();
        var value = (projectionCode == "none") ? null : projectionCode
        this.params.PROJECTION = value;

        return OpenLayers.Layer.Grid.prototype.getFullRequestString.apply(
                                                    this, arguments);
    },


    /**
     * Method: addTile
     * addTile creates a tile, initializes it and adds it to the
     * layer div.
     *
     * Parameters:
     * bounds - {<OpenLayers.Bounds>}
     * position - {<OpenLayers.Pixel>}
     *
     * Returns:
     * {<OpenLayers.Tile.InlineXhtml>} The added OpenLayers.Tile.InlineXhtml
     */
    addTile: function(bounds,position) {
        return new OpenLayers.Tile.InlineXhtml(
            this, position, bounds, this.url, this.tileSize);
    },

    CLASS_NAME: 'OpenLayers.Layer.InlineXhtml'
});
