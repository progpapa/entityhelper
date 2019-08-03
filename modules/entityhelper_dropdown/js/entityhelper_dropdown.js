/**
 * @file
 * Track changes on dropdowns and update entities.
 *
 * @todo: this whole file needs some thinking over...
 */
(function ($, Drupal, window, document, undefined) {

  Drupal.behaviors.entityhelper_dropdown = {
    attach: function (context, settings) {

      function entityhelper_dropdown_change() {
        $('.entityhelper-dropdown', context).on('change', function (e) {
          // Prevents the ajax call to fire multiple times.
//          e.stopImmediatePropagation();
          var value = $(this).val();
          if (entityhelper_dropdown_cardinality_failed($(this))) {
            return;
          }
          var entity_id = $(this).data('entityid');
          var entity_type = $(this).data('entitytype');
          var field_name = $(this).data('fieldname');
          var callbacks = $(this).data('callbacks');

          $.ajax({
            type: "POST",
            url: '/entityhelper/update-by-post/' + entity_type + '/' + entity_id,
            data: {"fields":{[field_name]:value}},
            success: function(data){
              entityhelper_dropdown_success_callback(
                data, entity_id, entity_type, field_name, callbacks, value);
            }
          });
        });
      }
      entityhelper_dropdown_change();

      // Calls all custom success callbacks defined on the "Manage Display"
      // screen for a field.
      function entityhelper_dropdown_success_callback(response, entity_id, entity_type, field_name, callbacks, value) {
        if (response !== 'success') {
          return;
        }
          if (typeof callbacks !== 'undefined') {
            var callback_arr = callbacks.split(',');
            $.each(callback_arr, function (index, callback) {
              var callbackParts = callback.split('.');
              var moduleName = callbackParts[0];
              var funcName = callbackParts[1];
              Drupal.behaviors[moduleName][funcName](entity_type, entity_id, field_name, value);
                });
          }
      }

      /**
       * Makes sure only the allowed number of options selected for select
       * lists. Also disables selection more than options than allowed.
       *
       * @param {Object} element
       *   The select list.
       *
       * @returns {Boolean}
       *   TRUE if the user tried to select more than the allowed number of
       *   options.
       */
      function entityhelper_dropdown_cardinality_failed(element) {
        var cardinality = element.data('cardinality');
        var value = element.val();
        var latest_values = element.data('last-selection');
        var length = Array.isArray(value) ? value.length : 1;
        if (cardinality !== -1 && length > cardinality) {
          element.once().before('<h4 class="fdropdown-warning">'
            + Drupal.t('You can select at most @cardinality options.'
            , {'@cardinality':cardinality})+'</h4>');
          element.val(latest_values);
          return true;
        }
        element.data('last-selection', value);
        return false;
      }
    },

    // Displays an "Updated" message next to a field that has been updated.
    entityhelper_dropdown_updated: function(entity_type, entity_id, field_name, value) {
      // Sorry...
      var selectSelector
        = $('*[data-entityid="' + entity_id + '"][data-entitytype="'
        + entity_type + '"][data-fieldname="' + field_name + '"]');
      $('<span class="fadeout-msg">' + Drupal.t('Updated') + '</span>')
        .insertAfter(selectSelector);
        $('.fadeout-msg').fadeOut(2000);
    }

  };
})(jQuery, Drupal, this, this.document);
